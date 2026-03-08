#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "./mathlib.c"

#define TILE_WIDTH 512
#define TILE_HEIGHT 512
#define TILESET_WIDTH (TILE_WIDTH * 4)
#define TILESET_HEIGHT (TILE_HEIGHT * 4)

typedef RGB (*Frag_Shader)(UV uv);
typedef uint32_t RGBA32;

RGBA32 make_rgba32(float r, float g, float b) 
{
  return (((uint32_t) (b * 255.f)) << 16) |
         (((uint32_t) (g * 255.f)) << 8 ) |
         (uint32_t) (r * 255.f) | 
         0xFF000000; 
}

typedef struct {
  size_t width; 
  size_t height; 
  size_t stride;
  uint32_t *pixels; 
} Tile;

RGB stripes(UV uv) 
{
  float n = 20.f; 
  Vec3f v = 
  {
    sinf(uv.c[U] * n), 
    sinf((uv.c[U] + uv.c[V]) * n),
    cosf(uv.c[V] * n)
  };
  
  return vec3f_mul(vec3f_sum(v, vec3fs(1.f)),
               vec3fs(0.5f));
}


void generate_image32(Tile *tile, Frag_Shader shader) 
{
  for (size_t y = 0; y < tile->height; ++y) 
  {
    for (size_t x = 0; x < tile->width; ++x) 
    {
      float u = (float) x / (float) tile->width;
      float v = (float) y / (float) tile->height;
      RGB p = shader(vec2f(u, v)); 
      tile->pixels[y * tile->stride + x] = make_rgba32(p.c[R], p.c[G], p.c[B]); 
    }
  }
}

int main() 
{
  uint32_t pixels[TILE_WIDTH * TILE_HEIGHT];

  Tile tile;
  tile.width = TILE_WIDTH;
  tile.height = TILE_HEIGHT;
  tile.stride = TILE_WIDTH; 
  tile.pixels = pixels;

  generate_image32(&tile, stripes); 
  
  const char *output_file_path = "output.png";  
  if (!stbi_write_png(output_file_path, tile.width, tile.height, 4, pixels, tile.width * sizeof(RGBA32)))
  {
    fprintf(stderr, "ERROR: could not save file %s: %s\n", output_file_path,
            strerror(errno));
    exit(1);
  } 
  
  return 0;
}
