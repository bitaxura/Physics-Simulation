#include <stdlib.h>
#include <math.h>
#include "physics.h"
#include "quadtree.h"

float GRAVITY = GRAVITY_ON;
int ball_count = 0;

Cell cells[NUM_CELLS][NUM_CELLS];
Ball balls[MAX_BALLS];

void spawn_ball(float x, float y) {
    balls[ball_count].position.x = x;
    balls[ball_count].position.y = y;

    balls[ball_count].velocity.x = ((rand() % 3) * 2 - 1) * BALL_INIT_VEL;
    balls[ball_count].velocity.y = ((rand() % 3) * 2 - 1) * BALL_INIT_VEL;

    balls[ball_count].radius = BALL_RADIUS;
    balls[ball_count].mass = BALL_MASS;

    balls[ball_count].color = generate_random_color();
    balls[ball_count].ball_number = ball_count;

    ball_count++;
}

void handle_box_collisions(Ball *ball) {
    if (ball->position.y + ball->radius > WINDOW_HEIGHT) {
        ball->position.y = WINDOW_HEIGHT - ball->radius;
        ball->velocity.y *= Y_DAMP;
    }
    if (ball->position.y - ball->radius < 0) {
        ball->position.y = ball->radius;
        ball->velocity.y *= Y_DAMP;
    }

    if (ball->position.x - ball->radius < 0) {
        ball->position.x = ball->radius;
        ball->velocity.x *= X_DAMP;
    }
    if (ball->position.x + ball->radius > WINDOW_WIDTH) {
        ball->position.x = WINDOW_WIDTH - ball->radius;
        ball->velocity.x *= X_DAMP;
    }
}

void handle_ball_to_ball_collision(Ball *ball1, Ball *ball2) {
    Vec rel_pos = vec_sub(ball1->position, ball2->position);
    float len2 = vec_len2(rel_pos);
    if (len2 <= 1.0e-8f) {
        return;
    }
    
    Vec rel_vel = vec_sub(ball1->velocity, ball2->velocity);
    float dot_prod = vec_dot(rel_vel, rel_pos);
    if (dot_prod >= 0.0f) {
        return;
    }
    
    float mass_sum = ball1->mass + ball2->mass;
    float mass_factor_ball1 = 2.0f * ball2->mass / mass_sum;
    float mass_factor_ball2 = 2.0f * ball1->mass / mass_sum;
    
    float inv_len2 = 1.0f / len2;
    ball1->velocity = vec_mul(vec_sub(ball1->velocity, vec_mul(rel_pos, mass_factor_ball1 * dot_prod * inv_len2)), FRICTION);
    ball2->velocity = vec_mul(vec_add(ball2->velocity, vec_mul(rel_pos, mass_factor_ball2 * dot_prod * inv_len2)), FRICTION);
}

void collision_check(Ball **cell_balls, int ball_count, float dt){
    for (int i = 0; i < ball_count; i++) {
        for (int j = i + 1; j < ball_count; j++){
            float dx = cell_balls[j]->position.x - cell_balls[i]->position.x;
            float dy = cell_balls[j]->position.y - cell_balls[i]->position.y;
            float radius_sum = cell_balls[i]->radius + cell_balls[j]->radius;
            float dist2 = dx * dx + dy * dy;
            // printf("collision checked between %d and %d\n", cell_balls[i]->ball_number, cell_balls[j]->ball_number);

            if (dist2 < radius_sum * radius_sum){
                float dist = sqrtf(dist2) + DIST_EPSILON;
                float overlap = radius_sum - dist;

                float nx = dx / dist;
                float ny = dy / dist;

                cell_balls[i]->position.x -= nx * overlap * OVERLAP_PERCENT;
                cell_balls[i]->position.y -= ny * overlap * OVERLAP_PERCENT;
                cell_balls[j]->position.x += nx * overlap * OVERLAP_PERCENT;
                cell_balls[j]->position.y += ny * overlap * OVERLAP_PERCENT;

                handle_ball_to_ball_collision(cell_balls[i], cell_balls[j]);
            }
        }
    }
}

void update_balls(float dt) {
    for (int i = 0; i < ball_count; i++) {
        balls[i].velocity.y += GRAVITY * dt;
        balls[i].position = vec_add(balls[i].position, vec_mul(balls[i].velocity, dt));
        handle_box_collisions(&balls[i]);
        // if (round(balls[i].velocity.y) == 0.0f) printf("%f\n", balls[i].position.y);
    }

    if (GRID_TYPE == 1){
        if (g_quadtree_root) {
            free_quad(g_quadtree_root);
            g_quadtree_root = NULL;
        }
        g_quadtree_root = create_node(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, 0);

        for (int i = 0; i < ball_count; i++){
            insert_ball_quad(g_quadtree_root, &balls[i]);
        }

        check_collision_quad(g_quadtree_root, dt);
    }

    if (GRID_TYPE == 0){
        insert_balls_static();
        
        for (int i = 0; i < NUM_CELLS; i++){
            for (int j = 0; j < NUM_CELLS; j++){
                collision_check(cells[i][j].balls, cells[i][j].ball_count, dt);
            }
        }
    }
}

void insert_balls_static(){
    for (int i = 0; i < NUM_CELLS; i++){
        for (int j = 0; j < NUM_CELLS; j++){
            cells[i][j].ball_count = 0;
        }
    }
    float cell_width = (WINDOW_WIDTH / NUM_CELLS);
    float cell_height = (WINDOW_HEIGHT / NUM_CELLS);
    for (int i = 0; i < ball_count; i++){
        int cell_x = (int)floor(balls[i].position.x / cell_width);
        int cell_y = (int)floor(balls[i].position.y / cell_height);

        if (cell_x < 0) cell_x = 0;
        if (cell_x >= NUM_CELLS) cell_x = NUM_CELLS - 1;
        if (cell_y < 0) cell_y = 0;
        if (cell_y >= NUM_CELLS) cell_y = NUM_CELLS - 1;

        cells[cell_x][cell_y].balls[cells[cell_x][cell_y].ball_count] = &balls[i];
        cells[cell_x][cell_y].ball_count++;
    }
}

void build_ball_partition() {
    float cell_width = (WINDOW_WIDTH / NUM_CELLS);
    float cell_height = (WINDOW_HEIGHT / NUM_CELLS);
    for (int i = 0; i < NUM_CELLS; i++){
        for (int j = 0; j < NUM_CELLS; j++){
            if (i < 0) i = 0;
            if (j < 0) j = 0;
            if (i >= NUM_CELLS) i = NUM_CELLS - 1;
            if (j >= NUM_CELLS) j = NUM_CELLS - 1;

            cells[i][j].x_min = i * cell_width;
            cells[i][j].x_max = (i + 1) * cell_width;
            cells[i][j].y_min = j * cell_height;
            cells[i][j].y_max = (j + 1) * cell_height;

            cells[i][j].ball_count = 0;
        }
    }
}