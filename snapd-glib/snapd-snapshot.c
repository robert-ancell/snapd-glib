/*
 * Copyright (C) 2019 Canonical Ltd.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 or version 3 of the License.
 * See http://www.gnu.org/copyleft/lgpl.html the full text of the license.
 */

#include "config.h"

#include "snapd-snapshot.h"

/**
 * SECTION: snapd-snapshot
 * @short_description: Snapshot information
 * @include: snapd-glib/snapd-glib.h
 *
 * A #SnapdSnapshot represents a snapshot of snap data.
 * Snapshots can be queried using snapd_client_get_snapshots().
 */

/**
 * SnapdSnapshot:
 *
 * #SnapdSnapshot contains snapshot information.
 *
 * Since: 1.50
 */

struct _SnapdSnapshot
{
    GObject parent_instance;

    gboolean auto_;
    gchar *revision;
    gchar *summary;
    gchar *snap;
    gchar *snap_id;
    gint64 size;
    GDateTime *time;
    gchar *version;
};

enum
{
    PROP_URL = 1,
    PROP_AUTO,
    PROP_REVISION,
    PROP_SUMMARY,
    PROP_SNAP,
    PROP_SNAP_ID,
    PROP_SIZE,
    PROP_TIME,
    PROP_VERSION,
    PROP_LAST
};

G_DEFINE_TYPE (SnapdSnapshot, snapd_snapshot, G_TYPE_OBJECT)

SnapdSnapshot *
snapd_snapshot_new (void)
{
    return g_object_new (SNAPD_TYPE_SNAPSHOT, NULL);
}

/**
 * snapd_snapshot_get_revision:
 * @snapshot: a #SnapdSnapshot.
 *
 * Get the revision of this snap that made this snapshot.
 *
 * Returns: a revision string.
 *
 * Since: 1.50
 */
const gchar *
snapd_snapshot_get_revision (SnapdSnapshot *self)
{
    g_return_val_if_fail (SNAPD_IS_SNAPSHOT (self), NULL);
    return self->revision;
}

/**
 * snapd_snapshot_get_auto:
 * @snapshot: a #SnapdSnapshot.
 *
 * Get if this snapshot was automatically made.
 *
 * Returns: %TRUE if this snapshot was automatically made.
 *
 * Since: 1.50
 */
gboolean
snapd_snapshot_get_auto (SnapdSnapshot *self)
{
    g_return_val_if_fail (SNAPD_IS_SNAPSHOT (self), FALSE);
    return self->auto_;
}

/**
 * snapd_snapshot_get_summary:
 * @snapshot: a #SnapdSnapshot.
 *
 * Get the summary for this snapshot.
 *
 * Returns: a summary string.
 *
 * Since: 1.50
 */
const gchar *
snapd_snapshot_get_summary (SnapdSnapshot *self)
{
    g_return_val_if_fail (SNAPD_IS_SNAPSHOT (self), NULL);
    return self->summary;
}

/**
 * snapd_snapshot_get_snap:
 * @snapshot: a #SnapdSnapshot.
 *
 * Get the snap that made this snapshot.
 *
 * Returns: a snap name.
 *
 * Since: 1.50
 */
const gchar *
snapd_snapshot_get_snap (SnapdSnapshot *self)
{
    g_return_val_if_fail (SNAPD_IS_SNAPSHOT (self), NULL);
    return self->snap;
}

/**
 * snapd_snapshot_get_snap_id:
 * @snapshot: a #SnapdSnapshot.
 *
 * Get the ID of the snap that made this snapshot.
 *
 * Returns: a snap id.
 *
 * Since: 1.50
 */
const gchar *
snapd_snapshot_get_snap_id (SnapdSnapshot *self)
{
    g_return_val_if_fail (SNAPD_IS_SNAPSHOT (self), NULL);
    return self->snap_id;
}

/**
 * snapd_snapshot_get_size:
 * @snapshot: a #SnapdSnapshot.
 *
 * Get the size of the snapshot in bytes.
 *
 * Return: a size
 *
 * Since: 1.50
 */
gint64
snapd_snapshot_get_size (SnapdSnapshot *self)
{
    g_return_val_if_fail (SNAPD_IS_SNAPSHOT (self), 0);
    return self->size;
}

/**
 * snapd_snapshot_get_time:
 * @snapshot: a #SnapdSnapshot.
 *
 * Get the time this snapshot was made or %NULL if unknown.
 *
 * Returns: (transfer none) (allow-none): a #GDateTime or %NULL.
 *
 * Since: 1.50
 */
GDateTime *
snapd_snapshot_get_time (SnapdSnapshot *self)
{
    g_return_val_if_fail (SNAPD_IS_SNAPSHOT (self), 0);
    return self->time;
}

/**
 * snapd_snapshot_get_version:
 * @snapshot: a #SnapdSnapshot.
 *
 * Get the version of the snap that made this snapshot.
 *
 * Returns: a version string.
 *
 * Since: 1.50
 */
const gchar *
snapd_snapshot_get_version (SnapdSnapshot *self)
{
    g_return_val_if_fail (SNAPD_IS_SNAPSHOT (self), NULL);
    return self->version;
}

static void
snapd_snapshot_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
    SnapdSnapshot *self = SNAPD_SNAPSHOT (object);

    switch (prop_id) {
    case PROP_AUTO:
        self->auto_ = g_value_get_boolean (value);
        break;
    case PROP_REVISION:
        g_free (self->revision);
        self->revision = g_strdup (g_value_get_string (value));
        break;
    case PROP_SUMMARY:
        g_free (self->summary);
        self->summary = g_strdup (g_value_get_string (value));
        break;
    case PROP_SNAP:
        g_free (self->snap);
        self->snap = g_strdup (g_value_get_string (value));
        break;
    case PROP_SNAP_ID:
        g_free (self->snap_id);
        self->snap_id = g_strdup (g_value_get_string (value));
        break;
    case PROP_SIZE:
        self->size = g_value_get_int64 (value);
        break;
    case PROP_TIME:
        g_clear_pointer (&self->time, g_date_time_unref);
        if (g_value_get_boxed (value) != NULL)
            self->time = g_date_time_ref (g_value_get_boxed (value));
        break;
    case PROP_VERSION:
        g_free (self->version);
        self->version = g_strdup (g_value_get_string (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
snapd_snapshot_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
    SnapdSnapshot *self = SNAPD_SNAPSHOT (object);

    switch (prop_id) {
    case PROP_AUTO:
        g_value_set_boolean (value, self->auto_);
        break;
    case PROP_REVISION:
        g_value_set_string (value, self->revision);
        break;
    case PROP_SUMMARY:
        g_value_set_string (value, self->summary);
        break;
    case PROP_SNAP:
        g_value_set_string (value, self->snap);
        break;
    case PROP_SNAP_ID:
        g_value_set_string (value, self->snap_id);
        break;
    case PROP_SIZE:
        g_value_set_int64 (value, self->size);
        break;
    case PROP_TIME:
        g_value_set_boxed (value, self->time);
        break;
    case PROP_VERSION:
        g_value_set_string (value, self->version);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
snapd_snapshot_finalize (GObject *object)
{
    SnapdSnapshot *self = SNAPD_SNAPSHOT (object);

    g_clear_pointer (&self->revision, g_free);
    g_clear_pointer (&self->summary, g_free);
    g_clear_pointer (&self->snap, g_free);
    g_clear_pointer (&self->snap_id, g_free);
    g_clear_pointer (&self->time, g_date_time_unref);
    g_clear_pointer (&self->version, g_free);

    G_OBJECT_CLASS (snapd_snapshot_parent_class)->finalize (object);
}

static void
snapd_snapshot_class_init (SnapdSnapshotClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = snapd_snapshot_set_property;
    gobject_class->get_property = snapd_snapshot_get_property;
    gobject_class->finalize = snapd_snapshot_finalize;

    g_object_class_install_property (gobject_class,
                                     PROP_AUTO,
                                     g_param_spec_boolean ("auto",
                                                           "auto",
                                                           "TRUE if this snapshot was automatically made",
                                                           FALSE,
                                                           G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
    g_object_class_install_property (gobject_class,
                                     PROP_REVISION,
                                     g_param_spec_string ("revision",
                                                          "revision",
                                                          "Revision of this snap that made this snapshot",
                                                          NULL,
                                                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
    g_object_class_install_property (gobject_class,
                                     PROP_SUMMARY,
                                     g_param_spec_string ("summary",
                                                          "summary",
                                                          "Summary for this snapshot",
                                                          NULL,
                                                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
    g_object_class_install_property (gobject_class,
                                     PROP_SNAP,
                                     g_param_spec_string ("snap",
                                                          "snap",
                                                          "The snap that made this snapshot",
                                                          NULL,
                                                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
    g_object_class_install_property (gobject_class,
                                     PROP_SNAP_ID,
                                     g_param_spec_string ("snap-id",
                                                          "snap-id",
                                                          "The ID of the snap that made this snapshot",
                                                          NULL,
                                                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
    g_object_class_install_property (gobject_class,
                                     PROP_SIZE,
                                     g_param_spec_int64 ("size",
                                                         "size",
                                                         "Size of snapshot in bytes",
                                                         G_MININT64, G_MAXINT64, 0,
                                                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
    g_object_class_install_property (gobject_class,
                                     PROP_TIME,
                                     g_param_spec_boxed ("time",
                                                         "time",
                                                         "Time this snapshot was made",
                                                         G_TYPE_DATE_TIME,
                                                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
    g_object_class_install_property (gobject_class,
                                     PROP_VERSION,
                                     g_param_spec_string ("version",
                                                          "version",
                                                          "The version of the snap that made this snapshot",
                                                          NULL,
                                                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}

static void
snapd_snapshot_init (SnapdSnapshot *self)
{
}
