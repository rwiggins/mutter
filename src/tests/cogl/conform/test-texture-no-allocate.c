#include <cogl/cogl.h>

#include "tests/cogl-test-utils.h"

/* Tests that the various texture types can be freed without being
 * allocated */

/* Texture size that is probably to big to fit within the texture
 * limits */
#define BIG_TEX_WIDTH 16384
#define BIG_TEX_HEIGHT 128

static void
test_texture_no_allocate (void)
{
  uint8_t *tex_data;
  CoglTexture *texture;
  CoglTexture *texture_2d;
  GError *error = NULL;

  tex_data = g_malloc (BIG_TEX_WIDTH * BIG_TEX_HEIGHT * 4);

  /* NB: if we make the atlas and sliced texture APIs public then this
   * could changed to explicitly use that instead of the magic texture
   * API */

  /* Try to create an atlas texture that is too big so it will
   * internally be freed without allocating */
  texture =
    cogl_atlas_texture_new_from_data (test_ctx,
                                      BIG_TEX_WIDTH,
                                      BIG_TEX_HEIGHT,
                                      /* format */
                                      COGL_PIXEL_FORMAT_RGBA_8888_PRE,
                                      /* rowstride */
                                      BIG_TEX_WIDTH * 4,
                                      tex_data,
                                      &error);

  g_free (tex_data);

  /* It's ok if this causes an error, we just don't want it to
   * crash */

  if (texture == NULL)
    g_error_free (error);
  else
    g_object_unref (texture);

  /* Try to create a sliced texture without allocating it */
  texture =
    cogl_texture_2d_sliced_new_with_size (test_ctx,
                                          BIG_TEX_WIDTH,
                                          BIG_TEX_HEIGHT,
                                          COGL_TEXTURE_MAX_WASTE);
  g_object_unref (texture);

  /* 2D texture */
  texture_2d = cogl_texture_2d_new_with_size (test_ctx,
                                              64, 64);
  g_object_unref (texture_2d);
}

COGL_TEST_SUITE (
  g_test_add_func ("/texture/no-allocate", test_texture_no_allocate);
)
