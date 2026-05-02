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

static inline Vec vec_add(Vec a, Vec b) {
    return (Vec){a.x + b.x, a.y + b.y};
}

static inline Vec vec_sub(Vec a, Vec b) {
    return (Vec){a.x - b.x, a.y - b.y};
}

static inline Vec vec_mul(Vec a, float b) {
    return (Vec){a.x * b, a.y * b};
}

static inline float vec_dot(Vec a, Vec b) {
    return (float)(a.x * b.x + a.y * b.y);
}

static inline float vec_len2(Vec a) {
    return vec_dot(a, a);
}

Color generate_random_color(void);

#endif // UTILS_H