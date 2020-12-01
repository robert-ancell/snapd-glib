/*
 * Copyright (C) 2020 Canonical Ltd.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 or version 3 of the License.
 * See http://www.gnu.org/copyleft/lgpl.html the full text of the license.
 */

#ifndef __SNAPD_SNAP_HEALTH_H__
#define __SNAPD_SNAP_HEALTH_H__

#if !defined(__SNAPD_GLIB_INSIDE__) && !defined(SNAPD_COMPILATION)
#error "Only <snapd-glib/snapd-glib.h> can be included directly."
#endif

#include <glib-object.h>

G_BEGIN_DECLS

#define SNAPD_TYPE_SNAP_HEALTH  (snapd_health_get_type ())

G_DECLARE_FINAL_TYPE (SnapdSnapHealth, snapd_health, SNAPD, SNAP_HEALTH, GObject)

const gchar *snapd_health_get_code    (SnapdSnapHealth *health);

const gchar *snapd_health_get_message (SnapdSnapHealth *health);

const gchar *snapd_health_get_status  (SnapdSnapHealth *health);

G_END_DECLS

#endif /* __SNAPD_SNAP_HEALTH_H__ */
