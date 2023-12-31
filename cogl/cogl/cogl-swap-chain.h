/*
 * Cogl
 *
 * A Low Level GPU Graphics and Utilities API
 *
 * Copyright (C) 2011 Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#if !defined(__COGL_H_INSIDE__) && !defined(COGL_COMPILATION)
#error "Only <cogl/cogl.h> can be included directly."
#endif

#include "cogl/cogl-types.h"

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _CoglSwapChain CoglSwapChain;

#define COGL_TYPE_SWAP_CHAIN (cogl_swap_chain_get_type ())

COGL_EXPORT
G_DECLARE_FINAL_TYPE (CoglSwapChain, cogl_swap_chain,
                      COGL, SWAP_CHAIN, GObject)


COGL_EXPORT CoglSwapChain *
cogl_swap_chain_new (void);

COGL_EXPORT void
cogl_swap_chain_set_has_alpha (CoglSwapChain *swap_chain,
                               gboolean has_alpha);

COGL_EXPORT void
cogl_swap_chain_set_length (CoglSwapChain *swap_chain,
                            int length);

G_END_DECLS
