#ifndef QUADTREE_H
#define QUADTREE_H

#include "physics.h"

#define MAX_BALLS_PER_NODE 256
#define MAX_LEVELS         16

typedef struct QuadNode{
    float x_min, x_max;
    float y_min, y_max;

    Ball *balls[MAX_BALLS_PER_NODE * 4];
    int ball_count;

    int level;
    struct QuadNode *children[4];
} QuadNode;

extern QuadNode* g_quadtree_root;

QuadNode* create_node(float x_min, float x_max, float y_min, float y_max, int level);
bool ball_in_node(QuadNode* node, Ball* ball);
void subdivide(QuadNode* node);
void insert_ball_quad(QuadNode* node, Ball* ball);
void check_collision_quad(QuadNode* node, float dt);
void free_quad(QuadNode* node);

#endif // QUADTREE_H