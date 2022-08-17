/*
 * Copyright (C) 2022 Canonical Ltd.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 or version 3 of the License.
 * See http://www.gnu.org/copyleft/lgpl.html the full text of the license.
 */

#include <gio/gunixsocketaddress.h>

#include "http-unix-session.h"

/* Number of bytes to read at a time */
#define READ_SIZE 1024

struct _HttpUnixSession
{
    GObject parent_instance;

    GMainContext *context;

    /* User agent to send to server */
    gchar *user_agent;

    /* Socket path to connect to */
    gchar *path;

    /* Socket to communicate with server */
    GSocket *socket;
    GSource *read_source;

    /* Open HTTP requests */
    GMutex requests_mutex;
    GPtrArray *requests;

    /* Data received from server */
    GMutex buffer_mutex;
    GByteArray *buffer;
    gsize n_read;
};

G_DEFINE_TYPE (HttpUnixSession, http_unix_session, G_TYPE_OBJECT)

static void
close_socket (HttpUnixSession *self)
{
    if (self->read_source != NULL)
       g_source_destroy (self->read_source);
    g_clear_pointer (&self->read_source, g_source_unref);
    if (self->socket != NULL)
        g_socket_close (self->socket, NULL);
    g_clear_object (&self->socket);
}

/* Check if we have all HTTP chunks */
static gboolean
have_chunked_body (const gchar *body, gsize body_length)
{
    while (TRUE) {
        /* Read chunk header, stopping on zero length chunk */
        const gchar *chunk_start = g_strstr_len (body, body_length, "\r\n");
        if (chunk_start == NULL)
            return FALSE;
        gsize chunk_header_length = chunk_start - body + 2;
        gsize chunk_length = strtoul (body, NULL, 16);
        if (chunk_length == 0)
            return TRUE;

        /* Check enough space for chunk body */
        gsize required_length = chunk_header_length + chunk_length + 2;
        if (required_length > body_length)
            return FALSE;
        // FIXME: Validate that \r\n is on the end of a chunk?
        body += required_length;
        body_length -= required_length;
    }
}

/* If more than one HTTP chunk, re-order buffer to contain one chunk.
 * Assumes body is a valid chunked data block (as checked with have_chunked_body()) */
static void
compress_chunks (gchar *body, gsize body_length, gchar **combined_start, gsize *combined_length, gsize *total_length)
{
    /* Use first chunk as output */
    *combined_length = strtoul (body, NULL, 16);
    *combined_start = strstr (body, "\r\n") + 2;
    if (*combined_length == 0) {
        *total_length = *combined_start - body;
        return;
    }

    /* Copy any remaining chunks beside the first one */
    gchar *chunk_start = *combined_start + *combined_length + 2;
    while (TRUE) {
        gsize chunk_length;

        chunk_length = strtoul (chunk_start, NULL, 16);
        chunk_start = strstr (chunk_start, "\r\n") + 2;
        if (chunk_length == 0) {
            *total_length = chunk_start - body;
            return;
        }

        /* Move this chunk on the end of the last one */
        memmove (*combined_start + *combined_length, chunk_start, chunk_length);
        *combined_length += chunk_length;

        chunk_start += chunk_length + 2;
    }
}

static void
complete_all_requests (HttpUnixSession *self, GError *error)
{
    g_autoptr(GMutexLocker) locker = g_mutex_locker_new (&self->requests_mutex);

    g_autoptr(GPtrArray) requests = self->requests;
    self->requests = g_ptr_array_new_with_free_func (g_object_unref);
    for (guint i = 0; i < requests->len; i++) {
        GTask *request = g_ptr_array_index (requests, i);
        g_autoptr(GError) e = g_error_copy (error);
        g_task_return_error (request, e);
    }
}

static gboolean
read_cb (GSocket *socket, GIOCondition condition, HttpUnixSession *self)
{
    g_autoptr(GMutexLocker) locker = g_mutex_locker_new (&self->buffer_mutex);

    if (self->n_read + READ_SIZE > self->buffer->len)
        g_byte_array_set_size (self->buffer, self->n_read + READ_SIZE);
    g_autoptr(GError) error = NULL;
    gssize n_read = g_socket_receive (socket,
                                      (gchar *) (self->buffer->data + self->n_read),
                                      READ_SIZE,
                                      NULL,
                                      &error);
    if (n_read < 0) {
        if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK))
            return TRUE;

        g_autoptr(GError) e = g_error_new (G_IO_ERROR,
                                           G_IO_ERROR_FAILED,
                                           "Failed to read from server: %s",
                                           error->message);
        complete_all_requests (self, e);
        return G_SOURCE_REMOVE;
    }

    self->n_read += n_read;

    while (TRUE) {
        /* Look for header divider */
        gchar *body = g_strstr_len ((gchar *) self->buffer->data, self->n_read, "\r\n\r\n");
        if (body == NULL)
            return G_SOURCE_CONTINUE;
        body += 4;
        gsize header_length = body - (gchar *) self->buffer->data;

        /* Match this response to the next uncompleted request */
        g_autoptr(GTask) request = NULL;
        {
            g_autoptr(GMutexLocker) locker = g_mutex_locker_new (&self->requests_mutex);
            if (self->requests->len == 0) {
                g_warning ("Ignoring unexpected response");
	        close_socket (self);
                return G_SOURCE_REMOVE;
            }
            request = g_object_ref (g_ptr_array_index (self->requests, 0));
        }
        SoupMessage *msg = g_task_get_task_data (request);

        /* Parse headers */
        g_clear_pointer (&msg->reason_phrase, g_free);
        if (!soup_headers_parse_response ((gchar *) self->buffer->data, header_length, msg->response_headers,
                                          NULL, &msg->status_code, &msg->reason_phrase)) {
            g_autoptr(GError) e = g_error_new (G_IO_ERROR,
                                               G_IO_ERROR_FAILED,
                                               "Failed to parse HTTP headers from server");
            complete_all_requests (self, e);
            return G_SOURCE_REMOVE;
        }

        /* Read content and process content */
        gsize content_length;
        g_autoptr(GBytes) message_body = NULL;
        switch (soup_message_headers_get_encoding (msg->response_headers)) {
        case SOUP_ENCODING_EOF:
            if (n_read != 0)
                return G_SOURCE_CONTINUE;

            content_length = self->n_read - header_length;
            message_body = g_bytes_new (body, content_length);
            break;

        case SOUP_ENCODING_CHUNKED:
            // FIXME: Find a way to abort on error
            if (!have_chunked_body (body, self->n_read - header_length))
                return G_SOURCE_CONTINUE;

            gchar *combined_start;
            gsize combined_length;
            compress_chunks (body, self->n_read - header_length, &combined_start, &combined_length, &content_length);
            message_body = g_bytes_new (combined_start, combined_length);
            break;

        case SOUP_ENCODING_CONTENT_LENGTH:
            content_length = soup_message_headers_get_content_length (msg->response_headers);
            if (self->n_read < header_length + content_length)
                return G_SOURCE_CONTINUE;

            message_body = g_bytes_new (body, content_length);
            break;

        default:
            {
                g_autoptr(GError) e = g_error_new (G_IO_ERROR,
                                                   G_IO_ERROR_FAILED,
                                                   "Unable to determine header encoding");
                complete_all_requests (self, e);
            }
            return G_SOURCE_REMOVE;
        }

        g_task_return_pointer (request, g_steal_pointer (&message_body), g_object_unref);
        g_ptr_array_remove (self->requests, request);

        /* Move remaining data to the start of the buffer */
        g_byte_array_remove_range (self->buffer, 0, header_length + content_length);
        self->n_read -= header_length + content_length;

        if (n_read == 0) {
            g_autoptr(GError) e = g_error_new (G_IO_ERROR,
                                               G_IO_ERROR_FAILED,
                                               "Server connection closed");
            complete_all_requests (self, e);
            return G_SOURCE_REMOVE;
        }
    }
}

static gboolean
open_socket (HttpUnixSession *self, GCancellable *cancellable, GError **error)
{
    if (self->socket != NULL)
        close_socket (self);

    g_autoptr(GError) error_local = NULL;
    self->socket = g_socket_new (G_SOCKET_FAMILY_UNIX,
                                 G_SOCKET_TYPE_STREAM,
                                 G_SOCKET_PROTOCOL_DEFAULT,
                                 &error_local);
    if (self->socket == NULL) {
        g_propagate_prefixed_error (error, error_local, "Unable to create HTTP socket");
        return FALSE;
    }
    g_socket_set_blocking (self->socket, FALSE);
    g_autoptr(GSocketAddress) address = g_unix_socket_address_new (self->path);
    if (!g_socket_connect (self->socket, address, cancellable, &error_local)) {
        g_propagate_prefixed_error (error, error_local, "Unable to connect HTTP socket");
        return FALSE;
    }

    self->read_source = g_socket_create_source (self->socket, G_IO_IN, NULL);
    g_source_set_callback (self->read_source, (GSourceFunc) read_cb, self, NULL);
    g_source_attach (self->read_source, self->context);

    return TRUE;
}

static void
append_string (GByteArray *array, const gchar *value)
{
    g_byte_array_append (array, (const guint8 *) value, strlen (value));
}

static gboolean
write_to_server (HttpUnixSession *self, GByteArray *data, GCancellable *cancellable, GError **error)
{
    guint n_sent = 0;
    while (n_sent < data->len) {
        gssize n_written = g_socket_send (self->socket, (const gchar *) (data->data + n_sent), data->len - n_sent, cancellable, error);
        if (n_written < 0)
            return FALSE;

        n_sent += n_written;
    }

    return TRUE;
}

HttpUnixSession *
http_unix_session_new (const gchar *path)
{
    HttpUnixSession *self = g_object_new (http_unix_session_get_type (), NULL);
    http_unix_session_set_path (self, path);
    return self;
}

void
http_unix_session_set_path (HttpUnixSession *self, const gchar *path)
{
    g_return_if_fail (HTTP_IS_UNIX_SESSION (self));

    g_free (self->path);
    self->path = g_strdup (path);
}

const gchar *
http_unix_session_get_path (HttpUnixSession *self)
{
    g_return_val_if_fail (HTTP_IS_UNIX_SESSION (self), NULL);
    return self->path;
}

void
http_unix_session_set_user_agent (HttpUnixSession *self, const gchar *user_agent)
{
    g_return_if_fail (HTTP_IS_UNIX_SESSION (self));

    g_free (self->user_agent);
    self->user_agent = g_strdup (user_agent);
}

const gchar *
http_unix_session_get_user_agent (HttpUnixSession *self)
{
    g_return_val_if_fail (HTTP_IS_UNIX_SESSION (self), NULL);
    return self->user_agent;
}

 void
http_unix_session_send_and_read_async (HttpUnixSession *self, SoupMessage *msg, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data)
{
    g_return_if_fail (HTTP_IS_UNIX_SESSION (self));
    g_return_if_fail (SOUP_IS_MESSAGE (msg));

    g_autoptr(GMutexLocker) locker = g_mutex_locker_new (&self->requests_mutex);

    g_autoptr(GTask) request = g_task_new (self, cancellable, callback, user_data);
    g_task_set_task_data (request, g_object_ref (msg), g_object_unref);

    soup_message_headers_append (msg->request_headers, "Host", "");
    soup_message_headers_append (msg->request_headers, "Connection", "keep-alive");
    if (self->user_agent != NULL)
        soup_message_headers_append (msg->request_headers, "User-Agent", self->user_agent);

    g_autoptr(GByteArray) request_data = g_byte_array_new ();
    append_string (request_data, msg->method);
    append_string (request_data, " ");
    SoupURI *uri = soup_message_get_uri (msg);
    append_string (request_data, uri->path);
    if (uri->query != NULL) {
        append_string (request_data, "?");
        append_string (request_data, uri->query);
    }
    append_string (request_data, " HTTP/1.1\r\n");
    SoupMessageHeadersIter iter;
    soup_message_headers_iter_init (&iter, msg->request_headers);
    const char *name, *value;
    while (soup_message_headers_iter_next (&iter, &name, &value)) {
        append_string (request_data, name);
        append_string (request_data, ": ");
        append_string (request_data, value);
        append_string (request_data, "\r\n");
    }
    append_string (request_data, "\r\n");

    g_autoptr(SoupBuffer) buffer = soup_message_body_flatten (msg->request_body);
    g_byte_array_append (request_data, (const guint8 *) buffer->data, buffer->length);

    gboolean new_socket = FALSE;
    if (self->socket == NULL) {
        g_autoptr(GError) error = NULL;
        if (!open_socket (self, cancellable, &error)) {
            g_task_return_error (request, error);
            return;
        }
        new_socket = TRUE;
    }

    /* Send HTTP request. If was re-using closed socket, then reconnect and retry */
    g_autoptr(GError) error = NULL;
    gboolean written = write_to_server (self, request_data, cancellable, &error);
    if (!written && !new_socket && g_error_matches (error, G_IO_ERROR, G_IO_ERROR_BROKEN_PIPE)) {
        g_clear_error (&error);

        if (!open_socket (self, cancellable, &error)) {
            g_task_return_error (request, error);
            return;
        }

        written = write_to_server (self, request_data, cancellable, &error);
    }
    if (!written) {
        g_prefix_error (&error, "Failed to write to server");
        g_task_return_error (request, error);
        return;
    }

    g_ptr_array_add (self->requests, g_object_ref (request));
}

GBytes *
http_unix_session_send_and_read_finish (HttpUnixSession *self, GAsyncResult *result, GError **error)
{
    g_return_val_if_fail (HTTP_IS_UNIX_SESSION (self), NULL);
    g_return_val_if_fail (g_task_is_valid (result, self), NULL);

    return g_task_propagate_pointer (G_TASK (result), error);
}

static void
http_unix_session_finalize (GObject *object)
{
    HttpUnixSession *self = HTTP_UNIX_SESSION (object);

    close_socket (self);

    g_clear_pointer (&self->context, g_main_context_unref);
    g_clear_pointer (&self->path, g_free);
    g_clear_pointer (&self->user_agent, g_free);
    g_clear_pointer (&self->requests, g_ptr_array_unref);

    G_OBJECT_CLASS (http_unix_session_parent_class)->finalize (object);
}

static void
http_unix_session_class_init (HttpUnixSessionClass *klass)
{
   GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

   gobject_class->finalize = http_unix_session_finalize;
}

static void
http_unix_session_init (HttpUnixSession *self)
{
    self->context = g_main_context_ref_thread_default ();
    self->requests = g_ptr_array_new_with_free_func (g_object_unref);
    self->buffer = g_byte_array_new ();
    g_mutex_init (&self->requests_mutex);
    g_mutex_init (&self->buffer_mutex);
}
