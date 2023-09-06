/*
 * Copyright (C) 2023 NVIDIA Corporation.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Written by:
 *     Austin Shafer <ashafer@nvidia.com>
 */

/**
 * MetaDrmTimeline
 *
 * MetaDrmTimeline is a helper for handling DRM syncobj operations. It
 * can import DRM syncobjs and export eventfds at a particular point.
 *
 * This is heavily inspired by wlroot's wlr_render_timeline, written by
 * Simon Ser.
 */

#include "config.h"

#include <xf86drm.h>
#ifdef HAVE_EVENTFD
#include <sys/eventfd.h>
#endif

#include "meta/util.h"
#include "wayland/meta-drm-timeline.h"

enum
{
  PROP_0,

  PROP_DRM_FD,
  PROP_SYNCOBJ_FD,

  N_PROPS
};

typedef struct _MetaDrmTimeline
{
  GObject parent;

  int drm;
  int drm_syncobj_fd;
  uint32_t drm_syncobj;
} MetaDrmTimeline;

static GParamSpec *obj_props[N_PROPS];

static void initable_iface_init (GInitableIface *initable_iface);

G_DEFINE_TYPE_WITH_CODE (MetaDrmTimeline, meta_drm_timeline, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE,
                                                initable_iface_init))

static void
meta_drm_timeline_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  MetaDrmTimeline *timeline = META_DRM_TIMELINE (object);

  switch (prop_id)
    {
    case PROP_DRM_FD:
      g_value_set_int (value, timeline->drm);
      break;
    case PROP_SYNCOBJ_FD:
      g_value_set_int (value, timeline->drm_syncobj_fd);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
meta_drm_timeline_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  MetaDrmTimeline *timeline = META_DRM_TIMELINE (object);
  int fd;

  switch (prop_id)
    {
    case PROP_DRM_FD:
      fd = g_value_get_int (value);
      timeline->drm = dup (fd);
      break;
    case PROP_SYNCOBJ_FD:
      fd = g_value_get_int (value);
      timeline->drm_syncobj_fd = dup (fd);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static gboolean
meta_drm_timeline_initable_init (GInitable     *initable,
                                 GCancellable  *cancellable,
                                 GError       **error)
{
  MetaDrmTimeline *timeline = META_DRM_TIMELINE (initable);

  if (drmSyncobjFDToHandle (timeline->drm, timeline->drm_syncobj_fd,
                            &timeline->drm_syncobj) != 0)
    {
      g_set_error (error, G_IO_ERROR,
                   G_IO_ERROR_FAILED,
        "Failed to import DRM syncobj");
      return FALSE;
    }

  return TRUE;
}

static void
initable_iface_init (GInitableIface *initable_iface)
{
  initable_iface->init = meta_drm_timeline_initable_init;
}

MetaDrmTimeline *
meta_drm_timeline_import_syncobj (int       fd,
                                  int       drm_syncobj,
                                  GError  **error)
{
  MetaDrmTimeline *timeline = g_initable_new (META_TYPE_DRM_TIMELINE,
                                              NULL, error,
                                              "drm-fd", fd,
                                              "syncobj-fd", drm_syncobj,
                                              NULL);

  return timeline;
}

int
meta_drm_timeline_get_eventfd (MetaDrmTimeline *timeline,
                               uint64_t         sync_point,
                               GError         **error)
{
  int fd = -1;
#ifdef HAVE_EVENTFD
  struct drm_syncobj_eventfd syncobj_eventfd;

  fd = eventfd (0, EFD_CLOEXEC);
  if (fd < 0)
    return -1;

  syncobj_eventfd = {
    .handle = timeline->drm_syncobj,
    .flags = 0,
    .point = sync_point,
    .fd = fd,
  };

  if (drmIoctl (timeline->drm, DRM_IOCTL_SYNCOBJ_EVENTFD, &syncobj_eventfd) != 0)
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                   "DRM_IOCTL_SYNCOBJ_EVENTFD: Failed to export eventfd");
      close (fd);
      return -1;
    }

#endif
  return fd;
}

int
meta_drm_timeline_set_sync_point (MetaDrmTimeline *timeline,
                                  uint64_t         sync_point,
                                  int              sync_fd,
                                  GError         **error)
{
  int fd = -1;
#ifdef HAVE_EVENTFD
  fd = eventfd (0, EFD_CLOEXEC);
  if (fd < 0)
    return -1;

  if (drmSyncobjImportSyncFileTimeline (timeline->drm, timeline->drm_syncobj,
                                        sync_fd, sync_point))
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                   "DRM_IOCTL_SYNCOBJ_IMPORT: Failed to import syncfd");
      close (fd);
      return -1;
    }

#endif
  return fd;
}

static void
meta_drm_timeline_finalize (GObject *object)
{
  MetaDrmTimeline *timeline = META_DRM_TIMELINE (object);

  drmSyncobjDestroy (timeline->drm, timeline->drm_syncobj);
  close (timeline->drm_syncobj_fd);
  close (timeline->drm);

  G_OBJECT_CLASS (meta_drm_timeline_parent_class)->finalize (object);
}

static void
meta_drm_timeline_init (MetaDrmTimeline *timeline)
{
  timeline->drm  = -1;
  timeline->drm_syncobj_fd = -1;
  timeline->drm_syncobj = -1;
}

static void
meta_drm_timeline_class_init (MetaDrmTimelineClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = meta_drm_timeline_get_property;
  object_class->set_property = meta_drm_timeline_set_property;
  object_class->finalize = meta_drm_timeline_finalize;

  obj_props[PROP_DRM_FD] =
    g_param_spec_object ("drm-fd",
                         "drm-fd",
                         "DRM fd",
                         FALSE,
                         G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS);

  obj_props[PROP_SYNCOBJ_FD] =
    g_param_spec_object ("syncobj-fd",
                         "syncobj-fd",
                         "DRM Syncobj fd",
                         FALSE,
                         G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, obj_props);
}
