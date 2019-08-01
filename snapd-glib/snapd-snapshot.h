/*
 * Copyright (C) 2019 Canonical Ltd.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 or version 3 of the License.
 * See http://www.gnu.org/copyleft/lgpl.html the full text of the license.
 */

#ifndef __SNAPD_SNAPSHOT_H__
#define __SNAPD_SNAPSHOT_H__

#if !defined(__SNAPD_GLIB_INSIDE__) && !defined(SNAPD_COMPILATION)
#error "Only <snapd-glib/snapd-glib.h> can be included directly."
#endif

#include <glib-object.h>

G_BEGIN_DECLS

#define SNAPD_TYPE_SNAPSHOT  (snapd_snapshot_get_type ())

G_DECLARE_FINAL_TYPE (SnapdSnapshot, snapd_snapshot, SNAPD, SNAPSHOT, GObject)

gboolean     snapd_snapshot_get_auto     (SnapdSnapshot *snapshot);

const gchar *snapd_snapshot_get_revision (SnapdSnapshot *snapshot);

const gchar *snapd_snapshot_get_summary  (SnapdSnapshot *snapshot);

const gchar *snapd_snapshot_get_snap     (SnapdSnapshot *snapshot);

const gchar *snapd_snapshot_get_snap_id  (SnapdSnapshot *snapshot);

gint64       snapd_snapshot_get_size     (SnapdSnapshot *snapshot);

GDateTime   *snapd_snapshot_get_time     (SnapdSnapshot *snapshot);

const gchar *snapd_snapshot_get_version  (SnapdSnapshot *snapshot);

G_END_DECLS

#endif /* __SNAPD_SNAPSHOT_H__ */
