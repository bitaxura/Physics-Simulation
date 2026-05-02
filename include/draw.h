#ifndef DRAW_H
#define DRAW_H

#include "utils.h"
#include "quadtree.h"

#define BALL_SEGMENTS       12
extern bool showGrid;
extern const unsigned int GRID_TYPE;
#define INDICES_COUNT      (BALL_SEGMENTS * 3)

void init_draw_utils(void);
void draw_ball(SDL_Renderer *renderer, float px, float py, int radius, Vec velocity, Color color);
void draw_quadtree(SDL_Renderer* renderer, QuadNode* node);
void render(SDL_Renderer *renderer);
void print_ball_count(void);

#endif // DRAW_Hi