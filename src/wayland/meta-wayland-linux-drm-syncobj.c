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

#include "config.h"

#include "meta/util.h"
#include "wayland/meta-wayland-private.h"
#include "wayland/meta-wayland-linux-drm-syncobj.h"
#include "backends/native/meta-backend-native-types.h"
#include "backends/native/meta-renderer-native.h"
#include "backends/native/meta-device-pool.h"

#include "linux-drm-syncobj-v1-server-protocol.h"

typedef struct _MetaWaylandDrmSyncobjManager
{
  GObject parent;

  // TODO: Needs multi-GPU handling?
  int drm;
} MetaWaylandDrmSyncobjManager;

typedef struct _MetaWaylandSyncobjSurface
{
  GObject parent;

  struct wl_resource *resource;
  MetaWaylandSurface *surface;
} MetaWaylandSyncobjSurface;

static void
syncobj_surface_handle_destroy (struct wl_client *client,
                             struct wl_resource *resource);
static void
syncobj_surface_handle_set_acquire_point (struct wl_client *client, struct wl_resource *resource,
                                       struct wl_resource *timeline_resource, uint32_t point_hi,
                                       uint32_t point_lo);
static void syncobj_surface_handle_set_release_point (struct wl_client *client,
                                                   struct wl_resource *resource,
                                                   struct wl_resource *timeline_resource,
                                                   uint32_t point_hi, uint32_t point_lo);
static void
syncobj_timeline_handle_destroy (struct wl_client *client,
                              struct wl_resource *resource);
static void
drm_syncobj_handle_destroy (struct wl_client *client,
                            struct wl_resource *resource);
static void
drm_syncobj_handle_get_surface (struct wl_client *client,
                                struct wl_resource *resource, uint32_t id,
                                struct wl_resource *surface_resource);
static void
drm_syncobj_handle_import_timeline (struct wl_client *client,
                                    struct wl_resource *resource,
                                    uint32_t id, int drm_syncobj_fd);

G_DEFINE_TYPE (MetaWaylandDrmSyncobjManager, meta_wayland_drm_syncobj_manager,
               G_TYPE_OBJECT)

G_DEFINE_TYPE (MetaWaylandSyncobjSurface, meta_wayland_syncobj_surface,
               G_TYPE_OBJECT)

G_DEFINE_TYPE (MetaWaylandSyncobjTimeline, meta_wayland_syncobj_timeline,
               G_TYPE_OBJECT)

static GQuark quark_syncobj_surface;

static const struct wp_linux_drm_syncobj_surface_v1_interface syncobj_surface_implementation =
{
  syncobj_surface_handle_destroy,
  syncobj_surface_handle_set_acquire_point,
  syncobj_surface_handle_set_release_point,
};

static const struct wp_linux_drm_syncobj_timeline_v1_interface syncobj_timeline_implementation =
{
  syncobj_timeline_handle_destroy,
};

static const struct wp_linux_drm_syncobj_v1_interface drm_syncobj_implementation =
{
  drm_syncobj_handle_destroy,
  drm_syncobj_handle_get_surface,
  drm_syncobj_handle_import_timeline,
};

static void
syncobj_timeline_handle_destroy (struct wl_client *client,
                              struct wl_resource *resource)
{
  wl_resource_destroy (resource);
}

static void
syncobj_surface_handle_destroy (struct wl_client *client,
                             struct wl_resource *resource)
{
  wl_resource_destroy (resource);
}

static void
syncobj_surface_handle_set_acquire_point (struct wl_client *client, struct wl_resource *resource,
                                       struct wl_resource *timeline_resource, uint32_t point_hi,
                                       uint32_t point_lo)
{
  MetaWaylandSyncobjSurface *syncobj_surface = wl_resource_get_user_data (resource);
  MetaWaylandSurface *surface = syncobj_surface->surface;
  MetaWaylandSyncobjTimeline *syncobj_timeline = wl_resource_get_user_data (timeline_resource);

  if (!syncobj_surface)
    {
      wl_resource_post_error (resource, WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_NO_SURFACE,
                              "Surface Sync object does not have surface attached");
      return;
    }

  if (!syncobj_timeline)
    {
      meta_topic (META_DEBUG_RENDER, "Invalid sync timeline object");
      return;
    }

  if (!surface->pending_state->drm_syncobj.acquire)
    surface->pending_state->drm_syncobj.acquire = g_object_new (META_TYPE_WAYLAND_SYNC_POINT, NULL);

  if (surface->pending_state->drm_syncobj.acquire->timeline)
    g_object_unref (surface->pending_state->drm_syncobj.acquire->timeline);

  surface->pending_state->drm_syncobj.acquire->timeline = syncobj_timeline;
  surface->pending_state->drm_syncobj.acquire->sync_point = (uint64_t)point_hi << 32 | point_lo;
}

static void syncobj_surface_handle_set_release_point (struct wl_client *client,
                                                   struct wl_resource *resource,
                                                   struct wl_resource *timeline_resource,
                                                   uint32_t point_hi, uint32_t point_lo)
{
  MetaWaylandSyncobjSurface *syncobj_surface = wl_resource_get_user_data (resource);
  MetaWaylandSurface *surface = syncobj_surface->surface;
  MetaWaylandSyncobjTimeline *syncobj_timeline = wl_resource_get_user_data (timeline_resource);

  if (!syncobj_surface)
    {
      wl_resource_post_error (resource, WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_NO_SURFACE,
                              "Surface Sync object does not have surface attached");
      return;
    }

  if (!syncobj_timeline)
    {
      meta_topic (META_DEBUG_RENDER, "Invalid sync timeline object");
      return;
    }

  if (!surface->pending_state->drm_syncobj.acquire)
    surface->pending_state->drm_syncobj.acquire = g_object_new (META_TYPE_WAYLAND_SYNC_POINT, NULL);

  if (surface->pending_state->drm_syncobj.release->timeline)
    {
      g_object_unref (surface->pending_state->drm_syncobj.release->timeline);
    }

  surface->pending_state->drm_syncobj.release->timeline = syncobj_timeline;
  surface->pending_state->drm_syncobj.release->sync_point = (uint64_t)point_hi << 32 | point_lo;
}

static void
drm_syncobj_handle_destroy (struct wl_client *client,
                            struct wl_resource *resource)
{
  wl_resource_destroy (resource);
}

static void
syncobj_surface_surface_destroyed_cb (gpointer user_data)
{
  MetaWaylandSyncobjSurface *surface_feedback = user_data;

  g_object_unref (surface_feedback);
}

static void
syncobj_surface_destructor (struct wl_resource *resource)
{
  MetaWaylandSyncobjSurface *syncobj_surface = wl_resource_get_user_data (resource);

  if (!syncobj_surface)
    return;

  g_object_set_qdata (G_OBJECT (syncobj_surface->surface),
                      quark_syncobj_surface, NULL);
}

static void
drm_syncobj_handle_get_surface (struct wl_client *client,
                                struct wl_resource *resource, uint32_t id,
                                struct wl_resource *surface_resource)
{
  MetaWaylandSurface *surface = wl_resource_get_user_data (surface_resource);
  MetaWaylandSyncobjSurface *syncobj_surface = g_object_get_qdata (G_OBJECT (surface),
                                                             quark_syncobj_surface);
  struct wl_resource *sync_resource;

  if (syncobj_surface)
    return;

  syncobj_surface = g_object_new (META_TYPE_WAYLAND_SYNCOBJ_SURFACE, NULL);
  syncobj_surface->surface = surface;
  g_object_set_qdata_full (G_OBJECT (surface),
                           quark_syncobj_surface,
                           syncobj_surface,
                           syncobj_surface_surface_destroyed_cb);

  sync_resource =
    wl_resource_create (client,
                        &wp_linux_drm_syncobj_surface_v1_interface,
                        wl_resource_get_version (resource),
                        id);
  if (sync_resource == NULL)
    {
      wl_resource_post_no_memory (resource);
      g_object_unref (syncobj_surface);
      return;
    }

  wl_resource_set_implementation (sync_resource,
                                  &syncobj_surface_implementation,
                                  syncobj_surface,
                                  syncobj_surface_destructor);
  syncobj_surface->resource = sync_resource;
}

static void
syncobj_timeline_handle_resource_destroy (struct wl_resource *resource)
{
}

static void
drm_syncobj_handle_import_timeline (struct wl_client *client,
                                    struct wl_resource *resource,
                                    uint32_t id, int drm_syncobj_fd)
{
  MetaWaylandDrmSyncobjManager *drm_syncobj = wl_resource_get_user_data (resource);
  GError *error = NULL;
  MetaDrmTimeline *drm_timeline = meta_drm_timeline_import_syncobj (drm_syncobj->drm,
                                                                    drm_syncobj_fd,
                                                                    &error);
  MetaWaylandSyncobjTimeline *syncobj_timeline;
  struct wl_resource *timeline_resource;

  close(drm_syncobj_fd);
  if (!drm_timeline)
    {
      wl_resource_post_error (resource, WP_LINUX_DRM_SYNCOBJ_V1_ERROR_INVALID_TIMELINE,
                              "Failed to import DRM syncobj: %s",
                              error ? error->message : "unknown error");
      return;
    }

  syncobj_timeline = g_object_new (META_TYPE_WAYLAND_SYNCOBJ_TIMELINE, NULL);

  timeline_resource = wl_resource_create (client, &wp_linux_drm_syncobj_timeline_v1_interface,
                                          wl_resource_get_version (resource), id);
  if (timeline_resource == NULL)
    {
      wl_resource_post_no_memory (resource);
      g_object_unref (drm_timeline);
      g_object_unref (syncobj_timeline);
      return;
    }

  wl_resource_set_implementation(timeline_resource,
      &syncobj_timeline_implementation, syncobj_timeline, syncobj_timeline_handle_resource_destroy);

  syncobj_timeline->resource = timeline_resource;
  syncobj_timeline->drm_timeline = drm_timeline;
}

static void
meta_wayland_syncobj_timeline_finalize (GObject *object)
{
  MetaWaylandSyncobjTimeline *syncobj_timeline =
    META_WAYLAND_SYNCOBJ_TIMELINE (object);

  g_object_unref (syncobj_timeline->drm_timeline);

  G_OBJECT_CLASS (meta_wayland_syncobj_timeline_parent_class)->finalize (object);
}

static void
meta_wayland_syncobj_timeline_class_init (MetaWaylandSyncobjTimelineClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = meta_wayland_syncobj_timeline_finalize;
}

static void
meta_wayland_syncobj_timeline_init (MetaWaylandSyncobjTimeline *syncobj_timeline)
{
  syncobj_timeline->resource = NULL;
  syncobj_timeline->drm_timeline = NULL;
}

static void
meta_wayland_syncobj_surface_finalize (GObject *object)
{
  G_OBJECT_CLASS (meta_wayland_syncobj_surface_parent_class)->finalize (object);
}

static void
meta_wayland_syncobj_surface_class_init (MetaWaylandSyncobjSurfaceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = meta_wayland_syncobj_surface_finalize;
}

static void
meta_wayland_syncobj_surface_init (MetaWaylandSyncobjSurface *syncobj_surface)
{
  syncobj_surface->resource = NULL;
  syncobj_surface->surface = NULL;
}

static void
meta_wayland_drm_syncobj_manager_finalize (GObject *object)
{
  MetaWaylandDrmSyncobjManager *drm_syncobj =
    META_WAYLAND_DRM_SYNCOBJ_MANAGER (object);

  close(drm_syncobj->drm);

  G_OBJECT_CLASS (meta_wayland_drm_syncobj_manager_parent_class)->finalize (object);
}

static void
meta_wayland_drm_syncobj_manager_class_init (MetaWaylandDrmSyncobjManagerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = meta_wayland_drm_syncobj_manager_finalize;
}

static void
meta_wayland_drm_syncobj_manager_init (MetaWaylandDrmSyncobjManager *drm_syncobj)
{
  drm_syncobj->drm = -1;
}

static void
drm_syncobj_bind (struct wl_client *client,
                  void             *user_data,
                  uint32_t          version,
                  uint32_t          id)
{
  MetaWaylandDrmSyncobjManager *drm_syncobj_manager = user_data;
  struct wl_resource *resource;

  resource = wl_resource_create (client, &wp_linux_drm_syncobj_v1_interface,
                                 version, id);
  wl_resource_set_implementation (resource, &drm_syncobj_implementation,
                                  drm_syncobj_manager, NULL);
}

void
meta_wayland_drm_syncobj_init (MetaWaylandCompositor *compositor)
{
#ifdef HAVE_NATIVE_BACKEND
  MetaContext *context =
    meta_wayland_compositor_get_context (compositor);
  MetaBackend *backend = meta_context_get_backend (context);
  MetaRenderer *renderer = meta_backend_get_renderer (backend);
  MetaRendererNative *renderer_native = META_RENDERER_NATIVE (renderer);
  MetaDeviceFile *device_file = meta_renderer_native_get_primary_device_file (renderer_native);
  g_autoptr (MetaWaylandDrmSyncobjManager) drm_syncobj_manager;

  if (!device_file)
    {
      meta_topic (META_DEBUG_RENDER, "Failed to get device file");
      return;
    }

  drm_syncobj_manager = g_object_new (META_TYPE_WAYLAND_DRM_SYNCOBJ_MANAGER, NULL);
  drm_syncobj_manager->drm = dup (meta_device_file_get_fd (device_file));

  if (!wl_global_create (compositor->wayland_display,
                         &wp_linux_drm_syncobj_v1_interface,
                         1,
                         drm_syncobj_manager,
                         drm_syncobj_bind))
    {
      g_error ("Failed to create wp_linux_drm_syncobjhronization_v1_interface global");
    }
#endif
}
