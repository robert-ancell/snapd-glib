/*
 * Copyright (C) 2016 Canonical Ltd.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 or version 3 of the License.
 * See http://www.gnu.org/copyleft/lgpl.html the full text of the license.
 */

#ifndef __SNAPD_TASK_H__
#define __SNAPD_TASK_H__

#if !defined(__SNAPD_GLIB_INSIDE__) && !defined(SNAPD_COMPILATION)
#error "Only <snapd-glib/snapd-glib.h> can be included directly."
#endif

#include <glib-object.h>

G_BEGIN_DECLS

#define SNAPD_TYPE_TASK  (snapd_task_get_type ())

G_DECLARE_FINAL_TYPE (SnapdTask, snapd_task, SNAPD, TASK, GObject)

/**
 * SnapdTaskStatus:
 * @SNAPD_TASK_STATUS_UNKNOWN: the status is unknown.
 * @SNAPD_TASK_STATUS_DO: the task is ready to start.
 * @SNAPD_TASK_STATUS_DOING: the task is in progress.
 * @SNAPD_TASK_STATUS_DONE: the task is complete.
 * @SNAPD_TASK_STATUS_ABORT: the task has been aborted.
 * @SNAPD_TASK_STATUS_UNDO: the task needs to be undone.
 * @SNAPD_TASK_STATUS_UNDOING: the task is being undone.
 * @SNAPD_TASK_STATUS_HOLD: the task will not be run (probably due to failure of another task).
 * @SNAPD_TASK_STATUS_ERROR: the task has completed with an error.
 *
 * Status codes for tasks.
 *
 * Since: 1.30
 */
typedef enum
{
    SNAPD_TASK_STATUS_UNKNOWN,
    SNAPD_TASK_STATUS_DO,
    SNAPD_TASK_STATUS_DOING,
    SNAPD_TASK_STATUS_DONE,
    SNAPD_TASK_STATUS_ABORT,
    SNAPD_TASK_STATUS_UNDO,
    SNAPD_TASK_STATUS_UNDOING,
    SNAPD_TASK_STATUS_HOLD,
    SNAPD_TASK_STATUS_ERROR
} SnapdTaskStatus;

const gchar    *snapd_task_get_id             (SnapdTask *task);

const gchar    *snapd_task_get_kind           (SnapdTask *task);

const gchar    *snapd_task_get_summary        (SnapdTask *task);

G_DEPRECATED_FOR (snapd_task_get_status_code)
const gchar    *snapd_task_get_status         (SnapdTask *task);

SnapdTaskStatus snapd_task_get_status_code    (SnapdTask *task);

G_DEPRECATED_FOR (snapd_change_get_ready)
gboolean        snapd_task_get_ready          (SnapdTask *task);

const gchar    *snapd_task_get_progress_label (SnapdTask *task);

gint64          snapd_task_get_progress_done  (SnapdTask *task);

gint64          snapd_task_get_progress_total (SnapdTask *task);

GDateTime      *snapd_task_get_spawn_time     (SnapdTask *task);

GDateTime      *snapd_task_get_ready_time     (SnapdTask *task);

G_END_DECLS

#endif /* __SNAPD_TASK_H__ */
