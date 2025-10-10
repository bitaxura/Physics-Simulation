#ifndef PHYSICS_H
#define PHYSICS_H

#include "utils.h"

#define MAX_BALLS       1000
#define NUM_CELLS       20
#define BALL_RADIUS     5.0f
#define BALL_MASS       1.0f
#define BALL_INIT_VEL   100.0f
#define X_DAMP          -0.25f
#define Y_DAMP          -0.75f
#define FRICTION         0.90f
#define OVERLAP_PERCENT  0.5f
#define DIST_EPSILON     0.05f

#define GRAVITY_OFF 0
#define GRAVITY_ON GRAVITY

extern int WINDOW_HEIGHT;
extern int WINDOW_WIDTH;
extern float GRAVITY;

extern int ball_count;

typedef struct {
    Vec position;
    Vec velocity;
    float radius;
    float mass;
    Color color;
} Ball;

typedef struct {
    float x_min, x_max;
    float y_min, y_max;
    Ball *balls[MAX_BALLS];
    int ball_count;
} Cell;

extern Cell cells[NUM_CELLS][NUM_CELLS];
extern Ball balls[MAX_BALLS];

void spawn_ball(float x, float y);
void handle_box_collisions(Ball *ball);
void handle_ball_to_ball_collision(Ball *ball1, Ball *ball2);
void collision_check(Ball **balls, int ball_count, float dt);
void update_balls(float dt);
void insert_balls_static(void);
void build_ball_partition(void);

#endif // PHYSICS_H