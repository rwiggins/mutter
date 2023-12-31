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
 *
 * Written by:
 *     Olivier Fourdan <ofourdan@redhat.com>
 */

#pragma once

#include <wayland-server.h>

#include "wayland/meta-wayland-types.h"
#include "meta/window.h"

#include "xwayland-keyboard-grab-unstable-v1-server-protocol.h"

#define META_TYPE_XWAYLAND_KEYBOARD_ACTIVE_GRAB (meta_xwayland_keyboard_active_grab_get_type ())
G_DECLARE_FINAL_TYPE (MetaXwaylandKeyboardActiveGrab,
                      meta_xwayland_keyboard_active_grab,
                      META, XWAYLAND_KEYBOARD_ACTIVE_GRAB,
                      GObject);

gboolean meta_xwayland_grab_keyboard_init (MetaWaylandCompositor *compositor);
