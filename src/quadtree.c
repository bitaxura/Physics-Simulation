#include "quadtree.h"
#include <stdlib.h>

QuadNode* g_quadtree_root = NULL;

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

void insert_ball_quad(QuadNode* node, Ball* ball){
    if (!node->children[0] && node->ball_count < MAX_BALLS_PER_NODE){
        node->balls[node->ball_count++] = ball;
        return;
    }

    if (!node->children[0] && node->level < MAX_LEVELS){
        subdivide(node);

        for (int i = 0; i < node->ball_count; i++){
            for (int j = 0; j < 4; j++){
                if (ball_in_node(node->children[j], node->balls[i])){
                    insert_ball_quad(node->children[j], node->balls[i]);
                    break;
                }
            }
        }
        node->ball_count = 0;
    }

    for (int i = 0; i < 4; i++){
        if (ball_in_node(node->children[i], ball)){
            insert_ball_quad(node->children[i], ball);
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