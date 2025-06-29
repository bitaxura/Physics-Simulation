#include <SDL3/SDL.h>
#include <stdlib.h>
#include <math.h>

#define MAX_BALLS 100
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

typedef struct {
    float x, y;
} vec;

typedef struct {
    vec position;
    vec velocity;
    float radius;
} Ball;

Ball balls[MAX_BALLS];
int ball_count = 0;

void spawn_ball(float x, float y){
    balls[ball_count].position.x = x;
    balls[ball_count].position.y = y;

    balls[ball_count].velocity.x =  (rand() % 3) * 2 - 1;
    balls[ball_count].velocity.y =  (rand() % 3) * 2 - 1;

    balls[ball_count].radius = 25.0;
    ball_count++;
}

void update_balls() {
    float gravity = 0;

    for (int i = 0; i < ball_count; i++) {
        balls[i].velocity.y += gravity;
        balls[i].position.x += balls[i].velocity.x;
        balls[i].position.y += balls[i].velocity.y;

        if (balls[i].position.y + balls[i].radius > WINDOW_HEIGHT){
            balls[i].position.y = WINDOW_HEIGHT - balls[i].radius;
            balls[i].velocity.y *= -1 * 0.7f;
        }
        if (balls[i].position.y - balls[i].radius < 0) {
            balls[i].position.y = balls[i].radius;
            balls[i].velocity.y *= -1 * 0.7f;
        }

        if (balls[i].position.x - balls[i].radius < 0) {
            balls[i].position.x = balls[i].radius;
            balls[i].velocity.x *= -1;
        }
        if (balls[i].position.x + balls[i].radius > WINDOW_WIDTH) {
            balls[i].position.x = WINDOW_WIDTH - balls[i].radius;
            balls[i].velocity.x *= -1;
        }

        for (int j = i + 1; j < ball_count; j++){
            float dx = balls[j].position.x - balls[i].position.x;
            float dy = balls[j].position.y - balls[i].position.y;
            float dist = sqrt(dx * dx + dy * dy);

            if (dist <= balls[i].radius + balls[j].radius){
                float tempvx = balls[i].velocity.x;
                float tempvy = balls[i].velocity.y;
                
                balls[i].velocity.x = balls[j].velocity.x;
                balls[i].velocity.y = balls[j].velocity.y;
                balls[j].velocity.x = tempvx;
                balls[j].velocity.y = tempvy;

                float overlap = balls[i].radius + balls[j].radius - dist;

                float nx = dx / dist;
                float ny = dy / dist;

                balls[i].position.x -= nx * overlap;
                balls[i].position.y -= ny * overlap;
                balls[j].position.x += nx * overlap;
                balls[j].position.y += ny * overlap;
            }
        }
    }
}

void draw_ball(SDL_Renderer *renderer, float px, float py, int radius){
    int cx = (int)px;
    int cy = (int)py;
    int r = radius;
    int r_squared = r*r;

    for (int y = -r; y <= r; y++){
        int x = (int)sqrt(r_squared - y*y);
        SDL_RenderLine(renderer, cx - x, cy + y, cx + x, cy + y);
    }
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

    window = SDL_CreateWindow("physics engine", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
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

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) quit = 1;
            else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && ball_count < MAX_BALLS){
                float mx, my;
                SDL_GetMouseState(&mx, &my);
                spawn_ball((float)mx, (float)my);
            }
        }
        update_balls();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        render_balls(renderer);        

        SDL_RenderPresent(renderer);
    }

    SDL_Log("SDL3 shutdown");
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}
