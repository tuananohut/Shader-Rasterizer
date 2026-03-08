#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../external/stb_image_write.h"

#include "./mathlib.c"

#define TILE_WIDTH 512
#define TILE_HEIGHT 512

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

RGB japan(UV uv) 
{
  float r = 0.25f; 
  int a = vec2f_sqrlen(vec2f_sub(vec2fs(0.5f), uv)) > r * r;  
  return vec3f(1.f, (float) a, (float) a);
}

RGB circles(UV uv) 
{
  float r1 = 0.25f; 
  float r2 = 0.30f; 
  float r3 = 0.35f; 
  
  float my_circle = vec2f_sqrlen(vec2f_sub(vec2fs(0.5f), uv));
  
  Vec3f color; 
  
  if (my_circle < r1*r1) 
  {
    color = vec3f(0.58f, 0.239f, 1.f); 
  } 
  else if (my_circle < r2*r2)
  {
    color = vec3f(0.596f, 0.165f, 0.416f);
  }
  else if (my_circle < r3*r3)
  {
    color = vec3f(0.f, 0.702f, 0.631f);
  }
  else 
    color = vec3f(0.094f, 0.094f, 0.094f);
  
  return color; 
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

int main(int argc, char *argv[]) 
{
  uint32_t pixels[TILE_WIDTH * TILE_HEIGHT];

  Tile tile;
  tile.width = TILE_WIDTH;
  tile.height = TILE_HEIGHT;
  tile.stride = TILE_WIDTH; 
  tile.pixels = pixels;
  
  Frag_Shader shader = stripes; 
  
  char *shader_names[] = 
  {
    "stripes", 
    "japan",
    "circles"
  };
  size_t shader_name_count = sizeof(shader_names) / sizeof(shader_names[0]);
  
  if (argc < 2) 
  {
    printf("This is Wang Tile generator. Pick a shader:\n");
    for (size_t i = 0; i < shader_name_count; ++i) 
    {
      printf("%s\n", shader_names[i]); 
    }   
    printf("Default stripes shader selected.\n");
  } 
  else 
  {
    char *shader_name = argv[1];
  
    if (strcmp(shader_name, "stripes") == 0)
      shader = stripes; 
    else if (strcmp(shader_name, "japan") == 0)
      shader = japan; 
    else if (strcmp(shader_name, "circles") == 0)
      shader = circles;
    else 
      fprintf(stderr, "Unknown shader: %s\n", shader_name);
  }
  
  generate_image32(&tile, shader); 
  
  const char *output_file_path = "../images/output.png";  
  if (!stbi_write_png(output_file_path, tile.width, tile.height, 4, pixels, tile.width * sizeof(RGBA32)))
  {
    fprintf(stderr, "ERROR: could not save file %s: %s\n", output_file_path,
            strerror(errno));
    exit(1);
  } 

  return 0;
}
