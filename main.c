#define _USE_MATH_DEFINES
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_BALLS           10000
#define BALL_RADIUS         5.0f
#define BALL_MASS           1.0f
#define COLOR_MAX           255
#define BALL_SPAWN_COUNT    48
#define BALL_INIT_VEL       300.0f
#define BALL_SEGMENTS       16

#define OVERLAP_PERCENT     0.5f
#define DIST_EPSILON        0.05f
#define GRAVITY_ON          980.0f
#define GRAVITY_OFF         0.0f
#define X_DAMP              -0.25f
#define Y_DAMP              -0.75f
#define FRICTION            0.90f

#define SIM_SPEED_STEP      0.5f

#define NUM_CELLS           20
#define MAX_BALLS_PER_NODE  64
#define MAX_LEVELS          11

float GRAVITY = GRAVITY_OFF;
int WINDOW_WIDTH = 1000;
int WINDOW_HEIGHT = 1000;
int ball_count = 0;
bool showGrid = false;

typedef struct {
    float x, y;
} Vec;

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

typedef struct {
    Uint8 r, g, b;
} Color;

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

typedef struct QuadNode{
    float x_min, x_max;
    float y_min, y_max;

    Ball *balls[MAX_BALLS_PER_NODE];
    int ball_count;

    int level;
    struct QuadNode *children[4];
} QuadNode;

Cell cells[NUM_CELLS][NUM_CELLS];
Ball balls[MAX_BALLS];

Color mix = {255, 255, 255};
QuadNode* g_quadtree_root = NULL;

Color generate_random_color() {
    int red = rand() % (COLOR_MAX + 1);
    int green = rand() % (COLOR_MAX + 1);
    int blue = rand() % (COLOR_MAX + 1);

    red = (red + mix.r) / 2;
    green = (green + mix.g) / 2;
    blue = (blue + mix.b) / 2;

    return (Color){red, green, blue};
}

void spawn_ball(float x, float y) {
    balls[ball_count].position.x = x;
    balls[ball_count].position.y = y;

    balls[ball_count].velocity.x = ((rand() % 3) * 2 - 1) * BALL_INIT_VEL;
    balls[ball_count].velocity.y = 0; // ((rand() % 3) * 2 - 1) * BALL_INIT_VEL;

    balls[ball_count].radius = BALL_RADIUS;
    balls[ball_count].mass = BALL_MASS;

    balls[ball_count].color = generate_random_color();

    ball_count++;
}

void handle_box_collisions(Ball *ball);
void handle_ball_to_ball_collision(Ball *ball1, Ball *ball2);
void collision_check(Ball **balls, int ball_count, float dt);

QuadNode* create_node(float x_min, float x_max, float y_min, float y_max, int level){
    QuadNode* node = malloc(sizeof(QuadNode));
    node->x_min = x_min;
    node->x_max = x_max;
    node->y_min = y_min;
    node->y_max = y_max;
    node->ball_count = 0;
    node->level = level;
    for (int i = 0; i < 4; i++){
        node->children[i] = NULL;
    }
    return node;
}

bool ball_in_node(QuadNode* node, Ball* ball){
    return (ball->position.x >= node->x_min &&
            ball->position.x <= node->x_max &&
            ball->position.y >= node->y_min &&
            ball->position.y <= node->y_max);
}

void subdivide(QuadNode* node){
    float mid_x = (node->x_min + node->x_max) * 0.5f;
    float mid_y = (node->y_min + node->y_max) * 0.5f;

    node->children[0] = create_node(node->x_min, mid_x, node->y_min, mid_y, node->level + 1);
    node->children[1] = create_node(mid_x, node->x_max, node->y_min, mid_y, node->level + 1);
    node->children[2] = create_node(node->x_min, mid_x, mid_y, node->y_max, node->level + 1);
    node->children[3] = create_node(mid_x, node->x_max, mid_y, node->y_max, node->level + 1);
}

void insert_ball(QuadNode* node, Ball* ball){
    if (!node->children[0] && node->ball_count < MAX_BALLS_PER_NODE){
        node->balls[node->ball_count++] = ball;
        return;
    }

    if (!node->children[0] && node->level < MAX_LEVELS){
        subdivide(node);

        for (int i = 0; i < node->ball_count; i++){
            for (int j = 0; j < 4; j++){
                if (ball_in_node(node->children[j], node->balls[i])){
                    insert_ball(node->children[j], node->balls[i]);
                    break;
                }
            }
        }
        node->ball_count = 0;
    }

    for (int i = 0; i < 4; i++){
        if (ball_in_node(node->children[i], ball)){
            insert_ball(node->children[i], ball);
            return;
        }
    }
}

void check_collision_quad(QuadNode* node, float dt){
    if (!node) return;

    if (!node->children[0]){
        collision_check(node->balls, node->ball_count, dt);
    }
    else {
        for (int i = 0; i < 4; i++){
            check_collision_quad(node->children[i], dt);
        }
    }
}

void free_quad(QuadNode* node){
    if (!node) return;

    for (int i = 0; i < 4; i++){
        if (node->children[i]){
            free_quad(node->children[i]);
            node->children[i] = NULL;
        }
    }

    free(node);
}

void draw_quadtree(SDL_Renderer* renderer, QuadNode* node) {
    if (!node) return;

    float x_min = node->x_min;
    float y_min = node->y_min;
    float x_max = node->x_max;
    float y_max = node->y_max;

    SDL_RenderLine(renderer, x_min, y_min, x_max, y_min);
    SDL_RenderLine(renderer, x_max, y_min, x_max, y_max);
    SDL_RenderLine(renderer, x_max, y_max, x_min, y_max);
    SDL_RenderLine(renderer, x_min, y_max, x_min, y_min);

    for (int i = 0; i < 4; i++) {
        draw_quadtree(renderer, node->children[i]);
    }
}


void update_balls(float dt) {
    if (g_quadtree_root) {
        free_quad(g_quadtree_root);
        g_quadtree_root = NULL;
    }
    g_quadtree_root = create_node(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, 0);

    for (int i = 0; i < ball_count; i++){
        insert_ball(g_quadtree_root, &balls[i]);
    }

    check_collision_quad(g_quadtree_root, dt);

    // for (int i = 0; i < NUM_CELLS; i++){
    //     for (int j = 0; j < NUM_CELLS; j++){
    //         cells[i][j].ball_count = 0;
    //     }
    // }
    // float cell_width = (WINDOW_WIDTH / NUM_CELLS);
    // float cell_height = (WINDOW_HEIGHT / NUM_CELLS);
    // for (int i = 0; i < ball_count; i++){
    //     int cell_x = (int)floor(balls[i].position.x / cell_width);
    //     int cell_y = (int)floor(balls[i].position.y / cell_height);

    //     if (cell_x < 0) cell_x = 0;
    //     if (cell_x >= NUM_CELLS) cell_x = NUM_CELLS - 1;
    //     if (cell_y < 0) cell_y = 0;
    //     if (cell_y >= NUM_CELLS) cell_y = NUM_CELLS - 1;

    //     cells[cell_x][cell_y].balls[cells[cell_x][cell_y].ball_count] = &balls[i];
    //     cells[cell_x][cell_y].ball_count++;
    // }

    // for (int i = 0; i < NUM_CELLS; i++){
    //     for (int j = 0; j < NUM_CELLS; j++){
    //         collision_check(cells[i][j].balls, cells[i][j].ball_count, dt);
    //     }
    // }
}

void collision_check(Ball **cell_balls, int ball_count, float dt){
    for (int i = 0; i < ball_count; i++) {
        cell_balls[i]->velocity.y += GRAVITY * dt;
        cell_balls[i]->position.x += cell_balls[i]->velocity.x * dt;
        cell_balls[i]->position.y += cell_balls[i]->velocity.y * dt;

        for (int j = i + 1; j < ball_count; j++){
            float dx = cell_balls[j]->position.x - cell_balls[i]->position.x;
            float dy = cell_balls[j]->position.y - cell_balls[i]->position.y;
            float dist = sqrtf(dx * dx + dy * dy) + DIST_EPSILON;

            if (dist < cell_balls[i]->radius + cell_balls[j]->radius){
                float overlap = cell_balls[i]->radius + cell_balls[j]->radius - dist;

                float nx = dx / dist;
                float ny = dy / dist;

                cell_balls[i]->position.x -= nx * overlap * OVERLAP_PERCENT;
                cell_balls[i]->position.y -= ny * overlap * OVERLAP_PERCENT;
                cell_balls[j]->position.x += nx * overlap * OVERLAP_PERCENT;
                cell_balls[j]->position.y += ny * overlap * OVERLAP_PERCENT;

                handle_ball_to_ball_collision(cell_balls[i], cell_balls[j]);
            }
        }
        handle_box_collisions(cell_balls[i]);
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

void handle_box_collisions(Ball *ball) {
    if (ball->position.y + ball->radius >= WINDOW_HEIGHT) {
        ball->position.y = WINDOW_HEIGHT - ball->radius;
        if (ball->velocity.y < 1) ball->velocity.y = 0;
        ball->velocity.y *= Y_DAMP;
    }
    if (ball->position.y - ball->radius <= 0) {
        ball->position.y = ball->radius;
        ball->velocity.y *= Y_DAMP;
    }

    if (ball->position.x - ball->radius <= 0) {
        ball->position.x = ball->radius;
        ball->velocity.x *= X_DAMP;
    }
    if (ball->position.x + ball->radius >= WINDOW_WIDTH) {
        ball->position.x = WINDOW_WIDTH - ball->radius;
        ball->velocity.x *= X_DAMP;
    }
}

void handle_ball_to_ball_collision(Ball *ball1, Ball *ball2) {
    Vec rel_pos_ball1 = vec_sub(ball1->position, ball2->position);
    Vec rel_pos_ball2 = vec_sub(ball2->position, ball1->position);

    float b1_len2 = vec_len2(rel_pos_ball1);
    float b2_len2 = vec_len2(rel_pos_ball2);

    Vec rel_vel_ball1 = vec_sub(ball1->velocity, ball2->velocity);
    Vec rel_vel_ball2 = vec_sub(ball2->velocity, ball1->velocity);

    float dot_prod_ball1 = vec_dot(rel_vel_ball1, rel_pos_ball1);
    float dot_prod_ball2 = vec_dot(rel_vel_ball2, rel_pos_ball2);

    float mass_factor_ball1 = 2.0f * ball1->mass / (ball1->mass + ball2->mass);
    float mass_factor_ball2 = 2.0f * ball2->mass / (ball1->mass + ball2->mass);

    Vec v1 = vec_sub(ball1->velocity, vec_mul(rel_pos_ball1, mass_factor_ball1 * dot_prod_ball1 / b1_len2));
    Vec v2 = vec_sub(ball2->velocity, vec_mul(rel_pos_ball2, mass_factor_ball2 * dot_prod_ball2 / b2_len2));

    ball1->velocity = vec_mul(v1, FRICTION);
    ball2->velocity = vec_mul(v2, FRICTION);
}

void draw_ball(SDL_Renderer *renderer, float px, float py, int radius, Vec velocity, Color color){
    const int segments = BALL_SEGMENTS;
    const int vertex_count = segments + 2;
    SDL_Vertex vertices[vertex_count];

    vertices[0].position.x = px;
    vertices[0].position.y = py;
    vertices[0].color.r = color.r / (float)COLOR_MAX;
    vertices[0].color.g = color.g / (float)COLOR_MAX;
    vertices[0].color.b = color.b / (float)COLOR_MAX;
    vertices[0].color.a = 1.0f;

    for (int i = 0; i <= segments; i++){
        float angle = ((float)i / (float)segments) * 2.0f * M_PI;
        float x = px + radius * cosf(angle);
        float y = py + radius * sinf(angle);

        vertices[i+1].position.x = x;
        vertices[i+1].position.y = y;
        vertices[i+1].color.r = color.r / (float)COLOR_MAX;
        vertices[i+1].color.g = color.g / (float)COLOR_MAX;
        vertices[i+1].color.b = color.b / (float)COLOR_MAX;
        vertices[i+1].color.a = 1.0f;

    // vertices[i+1].color = (SDL_FColor){255, 255, 255, 255};
    }

    const int indices_count = segments * 3;
    int indices[indices_count];

    for (int i = 0; i < segments; i++){
        indices[i*3] = 0;
        indices[i*3 + 1] = i + 1;
        indices[i*3 + 2] = i + 2;
    }

    SDL_RenderGeometry(renderer, NULL, vertices, vertex_count, indices, indices_count);

    // float x_start = px;
    // float x_end = (px + velocity.x);
    // float y_start = py;
    // float y_end = (py + velocity.y);
    // SDL_RenderLine(renderer, x_start, y_start, x_end, y_end);
}

void render(SDL_Renderer *renderer){
    for (int i = 0; i < ball_count; i++){
        draw_ball(renderer, balls[i].position.x, balls[i].position.y, balls[i].radius, balls[i].velocity, balls[i].color);
    }

    if (g_quadtree_root && showGrid) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        draw_quadtree(renderer, g_quadtree_root);
    }

    // if (showGrid){
    //     for (int i = 1; i < NUM_CELLS; i++){
    //         float x_value = cells[i][0].x_min;
    //         SDL_RenderLine(renderer, x_value, 0, x_value, WINDOW_HEIGHT);
    //         float y_value = cells[0][i].y_min;
    //         SDL_RenderLine(renderer, 0, y_value, WINDOW_WIDTH, y_value);
    //     }
    // }
}

void print_ball_count(){
    system("cls");
    for (int i = 0; i < NUM_CELLS; i++){
        for (int j = 0; j < NUM_CELLS; j++){
            printf("Cell (%d, %d) has %d balls\n", i+1, j+1, cells[i][j].ball_count);
        }
    }
}

int main() {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    int result1 = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    if (result1 < 0) {
        SDL_Log("SDL_Init error: %s", SDL_GetError());
        return -1;
    }

    window = SDL_CreateWindow("physics engine", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (window == NULL) {
        SDL_Log("SDL_CreateWindow: %s", SDL_GetError());
        return -2;
    }

    int CELL_SIZE_WIDTH = WINDOW_WIDTH / NUM_CELLS;
    int CELL_SIZE_HEIGHT = WINDOW_HEIGHT / NUM_CELLS;

    // build_ball_partition();

    renderer = SDL_CreateRenderer(window, "");
    if (renderer == NULL) {
        SDL_Log("SDL_CreateRenderer: %s", SDL_GetError());
        return -3;
    }

    int result2 = SDL_SetRenderVSync(renderer, true);
    if (result2 < 0){
        SDL_Log("Warning: Could not enable VSYNC: %s", SDL_GetError());
        return -4;
    }

    SDL_Log("SDL3 Initialized");

    SDL_Event event;
    int quit = 0;

    Uint64 freq = SDL_GetPerformanceFrequency();
    Uint64 prev = SDL_GetPerformanceCounter();
    float simulation_speed = 1.0f;
    const double max_dt = 1.0 / 60.0;
    double accumulator = 0.0;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) quit = 1;
            else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && ball_count < MAX_BALLS){
                float mx, my;
                SDL_GetMouseState(&mx, &my);
                for (int i = 0; i < BALL_SPAWN_COUNT; i++){
                    spawn_ball((float)mx, (float)my);
                    // printf("Ball color: (%d, %d, %d)\n", balls[ball_count - 1].color.r, balls[ball_count - 1].color.g, balls[ball_count - 1].color.b);
                }
                printf("%d ", ball_count);
            }
            else if (event.type == SDL_EVENT_WINDOW_RESIZED){
                int width, height;
                SDL_GetWindowSize(window, &width, &height);

                WINDOW_WIDTH = width;
                WINDOW_HEIGHT = height;
                CELL_SIZE_WIDTH = WINDOW_WIDTH / NUM_CELLS;
                CELL_SIZE_HEIGHT = WINDOW_HEIGHT / NUM_CELLS;

                // build_ball_partition();
            }
            else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_UP) {
                    simulation_speed += SIM_SPEED_STEP;
                    printf("Simulation speed: %.2f\n", simulation_speed);
                }
            else if (event.key.key == SDLK_DOWN) {
                if (simulation_speed > 0.0f){
                    simulation_speed -= SIM_SPEED_STEP;
                    printf("Simulation speed: %.2f\n", simulation_speed);
                }
            }
            else if (event.key.key == SDLK_BACKSPACE){
                if (ball_count >= BALL_SPAWN_COUNT) {
                    ball_count -= BALL_SPAWN_COUNT;
                    printf("Ball removed. Total balls: %d\n", ball_count);
                }
            }
            else if (event.key.key == SDLK_G){
                if (GRAVITY == GRAVITY_OFF) {
                    GRAVITY = GRAVITY_ON;
                    printf("Gravity enabled\n");
                } else {
                    GRAVITY = GRAVITY_OFF;
                    printf("Gravity disabled\n");
                }
            }
            else if (event.key.key == SDLK_ESCAPE) quit = 1;
            else if (event.key.key == SDLK_P){
                print_ball_count();
            }
            else if (event.key.key == SDLK_Q){
                showGrid = !showGrid;
            }
        }
    }

        Uint64 now = SDL_GetPerformanceCounter();
        double dt = (double)(now - prev) / (double)freq;
        prev = now;

        if (dt > 0.25) dt = 0.25;

        accumulator += dt * simulation_speed;

        while (accumulator >= max_dt) {
            update_balls(max_dt * simulation_speed);
            accumulator -= max_dt;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        render(renderer);

        SDL_RenderPresent(renderer);
    }

    if (g_quadtree_root) {
        free_quad(g_quadtree_root);
        g_quadtree_root = NULL;
    }

    SDL_Log("SDL3 shutdown");
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}
