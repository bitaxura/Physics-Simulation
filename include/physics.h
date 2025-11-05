#ifndef PHYSICS_H
#define PHYSICS_H

#include "utils.h"

#define MAX_BALLS       100000
#define NUM_CELLS       10
#define BALL_RADIUS     1.0f
#define BALL_MASS       1.0f
#define BALL_INIT_VEL   10.0f
#define X_DAMP          -0.95f
#define Y_DAMP          -0.95f
#define FRICTION         1.0f
#define OVERLAP_PERCENT  0.5f
#define DIST_EPSILON     0.05f

#define GRAVITY_OFF 0.0f
#define GRAVITY_ON 980.0f

extern int WINDOW_HEIGHT;
extern int WINDOW_WIDTH;
extern float GRAVITY;

extern int ball_count;

extern const char *GRID_TYPE;

typedef struct {
    Vec position;
    Vec velocity;
    float radius;
    float mass;
    Color color;
    int ball_number;
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