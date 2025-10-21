#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include "draw.h"

bool showGrid = true;

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
    
    if (strcmp(GRID_TYPE, "QUADTREE") == 0 && g_quadtree_root && showGrid) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        draw_quadtree(renderer, g_quadtree_root);
    }

    if (strcmp(GRID_TYPE, "STATIC") == 0 && showGrid){
        for (int i = 1; i < NUM_CELLS; i++){
            float x_value = cells[i][0].x_min;
            SDL_RenderLine(renderer, x_value, 0, x_value, WINDOW_HEIGHT);
            float y_value = cells[0][i].y_min;
            SDL_RenderLine(renderer, 0, y_value, WINDOW_WIDTH, y_value);
        }
    }
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

void print_ball_count(){
    system("cls");
    for (int i = 0; i < NUM_CELLS; i++){
        for (int j = 0; j < NUM_CELLS; j++){
            printf("Cell (%d, %d) has %d balls\n", i+1, j+1, cells[i][j].ball_count);
        }
    }
}