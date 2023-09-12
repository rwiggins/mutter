/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/*
 * Copyright (C) 2016 Red Hat Inc.
 * Copyright (C) 2017 Intel Corporation
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
 *     Jonas Ådahl <jadahl@gmail.com>
 *     Daniel Stone <daniels@collabora.com>
 */

#pragma once

#include <glib.h>
#include <glib-object.h>

#include "cogl/cogl.h"
#include "meta/meta-multi-texture.h"
#include "wayland/meta-wayland-types.h"

#define META_WAYLAND_DMA_BUF_MAX_FDS 4

#define META_TYPE_WAYLAND_DMA_BUF_BUFFER (meta_wayland_dma_buf_buffer_get_type ())
G_DECLARE_FINAL_TYPE (MetaWaylandDmaBufBuffer, meta_wayland_dma_buf_buffer,
                      META, WAYLAND_DMA_BUF_BUFFER, GObject);

#define META_TYPE_WAYLAND_DMA_BUF_MANAGER (meta_wayland_dma_buf_manager_get_type ())
G_DECLARE_FINAL_TYPE (MetaWaylandDmaBufManager, meta_wayland_dma_buf_manager,
                      META, WAYLAND_DMA_BUF_MANAGER, GObject)

typedef struct _MetaWaylandDmaBufBuffer
{
  GObject parent;

  MetaWaylandDmaBufManager *manager;

  int width;
  int height;
  uint32_t drm_format;
  uint64_t drm_modifier;
  bool is_y_inverted;
  int fds[META_WAYLAND_DMA_BUF_MAX_FDS];
  uint32_t offsets[META_WAYLAND_DMA_BUF_MAX_FDS];
  uint32_t strides[META_WAYLAND_DMA_BUF_MAX_FDS];
} MetaWaylandDmaBufBuffer;

MetaWaylandDmaBufManager * meta_wayland_dma_buf_manager_new (MetaWaylandCompositor  *compositor,
                                                             GError                **error);

gboolean
meta_wayland_dma_buf_buffer_attach (MetaWaylandBuffer  *buffer,
                                    MetaMultiTexture  **texture,
                                    GError            **error);

MetaWaylandDmaBufBuffer *
meta_wayland_dma_buf_fds_for_wayland_buffer (MetaWaylandBuffer *buffer);

MetaWaylandDmaBufBuffer *
meta_wayland_dma_buf_from_buffer (MetaWaylandBuffer *buffer);

CoglScanout *
meta_wayland_dma_buf_try_acquire_scanout (MetaWaylandDmaBufBuffer *dma_buf,
                                          CoglOnscreen            *onscreen);
