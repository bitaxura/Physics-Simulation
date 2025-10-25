#include <stdio.h>
#include <string.h>
#include "physics.h"
#include "draw.h"

#define BALL_SPAWN_COUNT    256
#define SIM_SPEED_STEP      0.5f
const char *GRID_TYPE = "QUADTREE";  // "STATIC" or "QUADTREE"

int WINDOW_WIDTH = 1000;
int WINDOW_HEIGHT = 1000;

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

    if (strcmp(GRID_TYPE, "STATIC") == 0) build_ball_partition();

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

    printf("Gravity: %.2f\n", GRAVITY);
    init_draw_utils();

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

                if (strcmp(GRID_TYPE, "STATIC") == 0) build_ball_partition();
            }
            else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_UP) {
                    simulation_speed += SIM_SPEED_STEP;
                    printf("Simulation speed: %.2f\n", simulation_speed);
                }
            else if (event.key.key == SDLK_LEFT) {
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
