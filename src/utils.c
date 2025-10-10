#include <stdlib.h>
#include "utils.h"

Color mix = {255, 255, 255};

Vec vec_add(Vec a, Vec b) {
    return (Vec){a.x + b.x, a.y + b.y};
}

Vec vec_sub(Vec a, Vec b) {
    return (Vec){a.x - b.x, a.y - b.y};
}

Vec vec_mul(Vec a, float b) {
    return (Vec){a.x * b, a.y * b};
}

float vec_dot(Vec a, Vec b) {
    return (float)(a.x * b.x + a.y * b.y);
}

float vec_len2(Vec a) {
    return vec_dot(a, a);
}

Color generate_random_color() {
    int red = rand() % (COLOR_MAX + 1);
    int green = rand() % (COLOR_MAX + 1);
    int blue = rand() % (COLOR_MAX + 1);

    red = (red + mix.r) / 2;
    green = (green + mix.g) / 2;
    blue = (blue + mix.b) / 2;

    return (Color){red, green, blue};
}