#ifndef UTILS_H
#define UTILS_H

#include <SDL3/SDL.h>

typedef struct {
    float x, y;
} Vec;

typedef struct {
    Uint8 r, g, b;
} Color;

#define COLOR_MAX 255
extern Color mix;

Vec vec_add(Vec a, Vec b);
Vec vec_sub(Vec a, Vec b);
Vec vec_mul(Vec a, float b);
float vec_dot(Vec a, Vec b);
float vec_len2(Vec a);

Color generate_random_color(void);

#endif // UTILS_H