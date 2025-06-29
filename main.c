#include <SDL3/SDL.h>
#include <stdio.h>

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
} ball;

ball balls[MAX_BALLS];
int ball_count = 0;

void spawn_ball(float x, float y){
    balls[ball_count].position.x = x;
    balls[ball_count].position.y = y;
    balls[ball_count].velocity.x = 1.0f;
    balls[ball_count].velocity.y = 1.0f;
    balls[ball_count].radius = 20;
    ball_count++;
}

void update_balls() {
    float gravity = 9.80f;

    for (int i = 0; i < ball_count; i++) {
        balls[i].velocity.y += gravity;
        balls[i].position.x += balls[i].velocity.x;
        balls[i].position.y += balls[i].velocity.y;

        if (balls[i].position.y + balls[i].radius > WINDOW_HEIGHT){
            balls[i].position.y = WINDOW_HEIGHT - balls[i].radius;
            balls[i].velocity.y *= -1;
        }
        
        if (balls[i].position.y - balls[i].radius < 0) {
            balls[i].position.y = balls[i].radius;
            balls[i].velocity.y *= -1;
        }

        if (balls[i].position.x - balls[i].radius < 0) {
            balls[i].position.x = balls[i].radius;
            balls[i].velocity.x *= -1;
        }
        if (balls[i].position.x + balls[i].radius > WINDOW_WIDTH) {
            balls[i].position.x = WINDOW_WIDTH - balls[i].radius;
            balls[i].velocity.x *= -1;
        }
    }
}

void render_balls(SDL_Renderer *renderer){
    for (int i = 0; i < ball_count; i++){
        for (int y = -(balls[i].radius); y <= (balls[i].radius); y++){
            for (int x = -(balls[i].radius); x <= (balls[i].radius); x++){
                if (x*x + y*y <= balls[i].radius * balls[i].radius){
                    SDL_RenderPoint(renderer, balls[i].position.x + x, balls[i].position.y + y);
                }
            }
        }
    }
}

int main() {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    int result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    if (result < 0) {
        SDL_Log("SDL_Init error: %s", SDL_GetError());
        return -1;
    }

    window = SDL_CreateWindow("physics engine", 800, 600, 0);
    if (window == NULL) {
        SDL_Log("SDL_CreateWindow: %s", SDL_GetError());
        return -2;
    }

    renderer = SDL_CreateRenderer(window, "");
    if (renderer == NULL) {
        SDL_Log("SDL_CreateRenderer: %s", SDL_GetError());
        return -3;
    }

    SDL_Log("SDL3 Initialized");

    SDL_Event event;
    int quit = 0;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) quit = 1;
            else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN){
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
        SDL_Delay(10);
    }

    SDL_Log("SDL3 shutdown");
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}
