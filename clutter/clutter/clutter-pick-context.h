/*
 * Copyright (C) 2019 Red Hat Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#if !defined(__CLUTTER_H_INSIDE__) && !defined(CLUTTER_COMPILATION)
#error "Only <clutter/clutter.h> can be included directly."
#endif

#include <glib-object.h>

#include "clutter/clutter-macros.h"
#include "clutter/clutter-stage-view.h"

typedef struct _ClutterPickContext ClutterPickContext;

#define CLUTTER_TYPE_PICK_CONTEXT (clutter_pick_context_get_type ())

CLUTTER_EXPORT
GType clutter_pick_context_get_type (void);

CLUTTER_EXPORT
ClutterPickContext * clutter_pick_context_ref (ClutterPickContext *pick_context);

CLUTTER_EXPORT
void clutter_pick_context_unref (ClutterPickContext *pick_context);

CLUTTER_EXPORT
void clutter_pick_context_destroy (ClutterPickContext *pick_context);

CLUTTER_EXPORT
ClutterPickMode clutter_pick_context_get_mode (ClutterPickContext *pick_context);

CLUTTER_EXPORT
void clutter_pick_context_log_pick (ClutterPickContext    *pick_context,
                                    const ClutterActorBox *box,
                                    ClutterActor          *actor);
CLUTTER_EXPORT
void clutter_pick_context_log_overlap (ClutterPickContext *pick_context,
                                       ClutterActor       *actor);

CLUTTER_EXPORT
void clutter_pick_context_push_clip (ClutterPickContext    *pick_context,
                                     const ClutterActorBox *box);

CLUTTER_EXPORT
void clutter_pick_context_pop_clip (ClutterPickContext *pick_context);

CLUTTER_EXPORT
void clutter_pick_context_push_transform (ClutterPickContext      *pick_context,
                                          const graphene_matrix_t *transform);

CLUTTER_EXPORT
void clutter_pick_context_get_transform (ClutterPickContext *pick_context,
                                         graphene_matrix_t  *out_matrix);

CLUTTER_EXPORT
void clutter_pick_context_pop_transform (ClutterPickContext *pick_context);
