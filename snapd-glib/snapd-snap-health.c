/*
 * Copyright (C) 2020 Canonical Ltd.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 or version 3 of the License.
 * See http://www.gnu.org/copyleft/lgpl.html the full text of the license.
 */

#include "snapd-snap-health.h"

/**
 * SECTION: snapd-snap-health
 * @short_description: Snap health status.
 * @include: snapd-glib/snapd-glib.h
 *
 * A #SnapdSnapHealth represents the health status of a snap. It can be queried
 * using snapd_snap_get_health().
 */

/**
 * SnapdSnapHealth:
 *
 * #SnapdSnapHealth contains health status of a snap.
 *
 * Since: 1.59
 */

struct _SnapdSnapHealth
{
    GObject parent_instance;

    gchar *code;
    gchar *message;
    gchar *status;  // FIXME: enum
};

enum
{
    PROP_CODE = 1,
    PROP_MESSAGE,
    PROP_STATUS,
    PROP_LAST
};

G_DEFINE_TYPE (SnapdSnapHealth, snapd_snap_health, G_TYPE_OBJECT)

/**
 * snapd_snap_health_get_code:
 * @price: a #SnapdSnapHealth.
 *
 * Get the status code for this snap health, e.g. "FIXME", or %NULL if not set.
 *
 * Returns: (allow-none): a health status code.
 *
 * Since: 1.59
 */
const gchar *
snapd_snap_health_get_code (SnapdSnapHealth *self)
{
    g_return_val_if_fail (SNAPD_IS_SNAP_HEALTH (self), NULL);
    return self->code;
}

/**
 * snapd_snap_health_get_message:
 * @price: a #SnapdSnapHealth.
 *
 * Get the message about this snap health, e.g. "FIXME", or %NULL if not set.
 *
 * Returns: (allow-none): a health status message.
 *
 * Since: 1.59
 */
const gchar *
snapd_snap_health_get_message (SnapdSnapHealth *self)
{
    g_return_val_if_fail (SNAPD_IS_SNAP_HEALTH (self), NULL);
    return self->message;
}

/**
 * snapd_snap_health_get_status:
 * @price: a #SnapdSnapHealth.
 *
 * Get the message about this snap health, e.g. "", or %NULL if not set.
 *
 * Returns: a health status message.
 *
 * Since: 1.59
 */
const gchar *
snapd_snap_health_get_status (SnapdSnapHealth *self)
{
    g_return_val_if_fail (SNAPD_IS_SNAP_HEALTH (self), NULL);
    return self->status;
}

static void
snapd_snap_health_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
    SnapdSnapHealth *self = SNAPD_SNAP_HEALTH (object);

    switch (prop_id) {
    case PROP_CODE:
        g_free (self->code);
        self->code = g_strdup (g_value_get_string (value));
        break;
    case PROP_MESSAGE:
        g_free (self->message);
        self->message = g_strdup (g_value_get_string (value));
        break;
    case PROP_STATUS:
        g_free (self->status);
        self->status = g_strdup (g_value_get_string (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
snapd_snap_health_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
    SnapdSnapHealth *self = SNAPD_SNAP_HEALTH (object);

    switch (prop_id) {
    case PROP_CODE:
        g_value_set_string (value, self->code);
        break;
    case PROP_MESSAGE:
        g_value_set_string (value, self->message);
        break;
    case PROP_STATUS:
        g_value_set_string (value, self->status);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
snapd_snap_health_finalize (GObject *object)
{
    SnapdSnapHealth *self = SNAPD_SNAP_HEALTH (object);

    g_clear_pointer (&self->code, g_free);
    g_clear_pointer (&self->message, g_free);
    g_clear_pointer (&self->status, g_free);

    G_OBJECT_CLASS (snapd_snap_health_parent_class)->finalize (object);
}

static void
snapd_snap_health_class_init (SnapdSnapHealthClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = snapd_snap_health_set_property;
    gobject_class->get_property = snapd_snap_health_get_property;
    gobject_class->finalize = snapd_snap_health_finalize;

    g_object_class_install_property (gobject_class,
                                     PROP_CODE,
                                     g_param_spec_string ("code",
                                                          "code",
                                                          "Health code",
                                                          NULL,
                                                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}

static void
snapd_snap_health_init (SnapdSnapHealth *self)
{
}
