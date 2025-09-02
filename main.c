#define _USE_MATH_DEFINES
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_BALLS 500
int WINDOW_WIDTH = 1200;
int WINDOW_HEIGHT = 900;

typedef struct {
    float x, y;
} vec;

typedef struct {
    vec position;
    vec velocity;
    float radius;
    float mass;
} Ball;

vec v_add(vec a, vec b){
    return (vec){a.x + b.x, a.y + b.y};
}

vec v_sub(vec a, vec b){
    return (vec){a.x - b.x, a.y - b.y};
}

vec v_mul(vec a, float b){
    return (vec){a.x * b, a.y * b};
}

float v_dot(vec a, vec b){
    return (float)(a.x * b.x + a.y * b.y);
}

float v_len2(vec a){
    return v_dot(a, a);
}

Ball balls[MAX_BALLS];
int ball_count = 0;

void spawn_ball(float x, float y){
    balls[ball_count].position.x = x;
    balls[ball_count].position.y = y;

    balls[ball_count].velocity.x =  ((rand() % 3) * 2 - 1) * 10;
    balls[ball_count].velocity.y =  ((rand() % 3) * 2 - 1) * 10;

    balls[ball_count].radius = 25.0f;
    balls[ball_count].mass = fabs((rand() % 3) * 2 - 1);
    ball_count++;
}

void handle_box_collisions(Ball *ball);
void handle_ball_to_ball_collision(Ball *ball1, Ball *ball2);

void update_balls(float dt) {
    float gravity = 0.0f;

    for (int i = 0; i < ball_count; i++) {
        balls[i].velocity.y += gravity * dt;
        balls[i].position.x += balls[i].velocity.x * dt;
        balls[i].position.y += balls[i].velocity.y * dt;

        handle_box_collisions(&balls[i]);

        for (int j = i + 1; j < ball_count; j++){
            float dx = balls[j].position.x - balls[i].position.x;
            float dy = balls[j].position.y - balls[i].position.y;
            float dist = sqrt(dx * dx + dy * dy) + 0.1f;

            float percent = 0.5f;

            if (dist < balls[i].radius + balls[j].radius){
                float overlap = balls[i].radius + balls[j].radius - dist;

                float nx = dx / dist;
                float ny = dy / dist;

                balls[i].position.x -= nx * overlap * percent;
                balls[i].position.y -= ny * overlap * percent;
                balls[j].position.x += nx * overlap * percent;
                balls[j].position.y += ny * overlap * percent;

                handle_ball_to_ball_collision(&balls[i], &balls[j]);
            }
        }
    }
}

void handle_box_collisions(Ball *ball) {
    if (ball->position.y + ball->radius > WINDOW_HEIGHT) {
        ball->position.y = WINDOW_HEIGHT - ball->radius;

        ball->velocity.y *= -1/2.0f;
    }
    if (ball->position.y - ball->radius < 0) {
        ball->position.y = ball->radius;
        if (ball->velocity.y < 1) ball->velocity.y = 0;
        ball->velocity.y *= -1/2.0f;
    }

    if (ball->position.x - ball->radius < 0) {
        ball->position.x = ball->radius;
        ball->velocity.x *= -1/2.0f;
    }
    if (ball->position.x + ball->radius > WINDOW_WIDTH) {
        ball->position.x = WINDOW_WIDTH - ball->radius;
        ball->velocity.x *= -1/2.0f;
    }
}

void handle_ball_to_ball_collision(Ball *ball1, Ball *ball2){
    vec rel_pos_ball1 = v_sub(ball1->position, ball2->position);
    vec rel_pos_ball2 = v_sub(ball2->position, ball1->position);

    float b1_len2 = v_len2(rel_pos_ball1);
    float b2_len2 = v_len2(rel_pos_ball2);

    vec rel_vel_ball1 = v_sub(ball1->velocity, ball2->velocity);
    vec rel_vel_ball2 = v_sub(ball2->velocity, ball1->velocity);

    float dot_prod_ball1 = v_dot(rel_vel_ball1, rel_pos_ball1);
    float dot_prod_ball2 = v_dot(rel_vel_ball2, rel_pos_ball2);

    float mass_factor_ball1 = 2.0f * ball1->mass / (ball1->mass + ball2->mass);
    float mass_factor_ball2 = 2.0f * ball2->mass / (ball1->mass + ball2->mass);

    ball1->velocity = v_sub(ball1->velocity, v_mul(rel_pos_ball1, mass_factor_ball1 * dot_prod_ball1 / b1_len2));
    ball2->velocity = v_sub(ball2->velocity, v_mul(rel_pos_ball2, mass_factor_ball2 * dot_prod_ball2 / b2_len2));
}

void draw_ball(SDL_Renderer *renderer, float px, float py, int radius){
    const int segments = 32;
    const int vertex_count = segments + 2;
    SDL_Vertex vertices[vertex_count];

    vertices[0].position.x = px;
    vertices[0].position.y = py;
    vertices[0].color = (SDL_FColor){255, 255, 255, 255};

    for (int i = 0; i<=segments; i++){
        float angle = ((float)i / (float)segments) * 2.0f * M_PI;
        float x = px + radius * cosf(angle);
        float y = py + radius * sinf(angle);

        vertices[i+1].position.x = x;
        vertices[i+1].position.y = y;
        vertices[i+1].color = (SDL_FColor){255, 255, 255, 255};
    }

    const int indices_count = segments * 3;
    int indices[indices_count];

    for (int i = 0; i < segments; i++){
        indices[i*3] = 0;
        indices[i*3 + 1] = i + 1;
        indices[i*3 + 2] = i + 2;
    }

    SDL_RenderGeometry(renderer, NULL, vertices, vertex_count, indices, indices_count);
}

void render_balls(SDL_Renderer *renderer){
    for (int i = 0; i < ball_count; i++){
        draw_ball(renderer, balls[i].position.x, balls[i].position.y, balls[i].radius);
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

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) quit = 1;
            else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && ball_count < MAX_BALLS){
                float mx, my;
                SDL_GetMouseState(&mx, &my);
                for (int i = 0; i < 10; i++){
                    spawn_ball((float)mx, (float)my);
                }
                printf("%d ", ball_count);
            } 
            else if (event.type == SDL_EVENT_WINDOW_RESIZED){
                int width, height;
                SDL_GetWindowSize(window, &width, &height);
                WINDOW_WIDTH = width;
                WINDOW_HEIGHT = height;
            }
            else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_UP) {
                    simulation_speed += 0.5f;
                    printf("Simulation speed: %.2f\n", simulation_speed);
                } 
            else if (event.key.key == SDLK_DOWN) {
                if (simulation_speed > 0.0f){
                    simulation_speed -= 0.5f;
                    printf("Simulation speed: %.2f\n", simulation_speed);
                }
            }
            else if (event.key.key == SDLK_BACKSPACE){
                if (ball_count >= 10) {
                    ball_count-=10;
                    printf("Ball removed. Total balls: %d\n", ball_count);
                }
            }
        }
    }

        Uint64 now = SDL_GetPerformanceCounter();
        double dt = (double)(now - prev) / (double)freq;
        prev = now;
        if (dt > 1.0/60.0) dt = 1.0/60.0;

        update_balls(dt * simulation_speed);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        render_balls(renderer);        

        SDL_RenderPresent(renderer);
    }

    SDL_Log("SDL3 shutdown");
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}
