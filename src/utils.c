#include <stdlib.h>
#include "utils.h"

Color mix = {255, 255, 255};

Color generate_random_color() {
    int red = rand() % (COLOR_MAX + 1);
    int green = rand() % (COLOR_MAX + 1);
    int blue = rand() % (COLOR_MAX + 1);

    red = (red + mix.r) / 2;
    green = (green + mix.g) / 2;
    blue = (blue + mix.b) / 2;

    return (Color){red, green, blue};
}