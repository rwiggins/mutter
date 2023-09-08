/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

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

#pragma once

#include <glib.h>

#include "wayland/meta-wayland-types.h"
#include "wayland/meta-drm-timeline.h"

#define META_TYPE_WAYLAND_DRM_SYNCOBJ_MANAGER (meta_wayland_drm_syncobj_manager_get_type ())
G_DECLARE_FINAL_TYPE (MetaWaylandDrmSyncobjManager, meta_wayland_drm_syncobj_manager,
                      META, WAYLAND_DRM_SYNCOBJ_MANAGER, GObject)

#define META_TYPE_WAYLAND_SYNCOBJ_SURFACE (meta_wayland_syncobj_surface_get_type ())
G_DECLARE_FINAL_TYPE (MetaWaylandSyncobjSurface, meta_wayland_syncobj_surface,
                      META, WAYLAND_SYNCOBJ_SURFACE, GObject)

#define META_TYPE_WAYLAND_SYNCOBJ_TIMELINE (meta_wayland_syncobj_timeline_get_type ())
G_DECLARE_FINAL_TYPE (MetaWaylandSyncobjTimeline, meta_wayland_syncobj_timeline,
                      META, WAYLAND_SYNCOBJ_TIMELINE, GObject)

typedef struct _MetaWaylandSyncobjSurface MetaWaylandSyncobjSurface;

typedef struct _MetaWaylandSyncobjTimeline
{
  GObject parent;

  struct wl_resource *resource;
  MetaDrmTimeline *drm_timeline;
} MetaWaylandSyncobjTimeline;

typedef void (*MetaWaylandDrmSyncobjSourceDispatch) (MetaWaylandSurface *surface,
                                                     gpointer            user_data);

void meta_wayland_drm_syncobj_init (MetaWaylandCompositor *compositor);

GSource *
meta_wayland_drm_syncobj_create_source (MetaWaylandSurface                    *surface,
                                        MetaWaylandSyncobjTimeline               *timeline,
                                        uint64_t                               sync_point,
                                        MetaWaylandDrmSyncobjSourceDispatch  dispatch,
                                        gpointer                               user_data);
