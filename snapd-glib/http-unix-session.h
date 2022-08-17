/*
 * Copyright (C) 2022 Canonical Ltd.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 or version 3 of the License.
 * See http://www.gnu.org/copyleft/lgpl.html the full text of the license.
 */

#ifndef __HTTP_UNIX_SESSION_H__
#define __HTTP_UNIX_SESSION_H__

#include <glib-object.h>
#include <libsoup/soup.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE (HttpUnixSession, http_unix_session, HTTP, UNIX_SESSION, GObject)

HttpUnixSession *http_unix_session_new                  (const gchar *path);

void             http_unix_session_set_path             (HttpUnixSession *session,
                                                         const gchar *path);

const gchar     *http_unix_session_get_path             (HttpUnixSession *session);

void             http_unix_session_set_user_agent       (HttpUnixSession *session,
                                                         const gchar *user_agent);

const gchar     *http_unix_session_get_user_agent       (HttpUnixSession *session);

void             http_unix_session_send_and_read_async  (HttpUnixSession *session,
                                                         SoupMessage *msg,
                                                         int io_priority,
                                                         GCancellable *cancellable,
                                                         GAsyncReadyCallback callback,
                                                         gpointer user_data);

GBytes          *http_unix_session_send_and_read_finish (HttpUnixSession       *session,
                                                         GAsyncResult          *result,
                                                         GError               **error);

G_END_DECLS

#endif /* __HTTP_UNIX_SESSION_H__ */
