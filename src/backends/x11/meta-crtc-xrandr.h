/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/*
 * Copyright (C) 2017 Red Hat
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
 */

#pragma once

#include <X11/extensions/Xrandr.h>
#include <xcb/randr.h>

#include "backends/meta-crtc.h"
#include "backends/x11/meta-gpu-xrandr.h"

#define META_TYPE_CRTC_XRANDR (meta_crtc_xrandr_get_type ())
G_DECLARE_FINAL_TYPE (MetaCrtcXrandr, meta_crtc_xrandr,
                      META, CRTC_XRANDR,
                      MetaCrtc)

gboolean meta_crtc_xrandr_set_config (MetaCrtcXrandr      *crtc_xrandr,
                                      xcb_randr_crtc_t     xrandr_crtc,
                                      xcb_timestamp_t      timestamp,
                                      int                  x,
                                      int                  y,
                                      xcb_randr_mode_t     mode,
                                      xcb_randr_rotation_t rotation,
                                      xcb_randr_output_t  *outputs,
                                      int                  n_outputs,
                                      xcb_timestamp_t     *out_timestamp);

gboolean meta_crtc_xrandr_is_assignment_changed (MetaCrtcXrandr     *crtc_xrandr,
                                                 MetaCrtcAssignment *crtc_assignment);

MetaCrtcMode * meta_crtc_xrandr_get_current_mode (MetaCrtcXrandr *crtc_xrandr);

MetaCrtcXrandr * meta_crtc_xrandr_new (MetaGpuXrandr      *gpu_xrandr,
                                       XRRCrtcInfo        *xrandr_crtc,
                                       RRCrtc              crtc_id,
                                       XRRScreenResources *resources);
