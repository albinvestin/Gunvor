#ifndef PERSON_H
#define PERSON_H

#include <stdbool.h>
#include "../../common/VectorUtils3.h"

struct Person {
    vec3 position;
    int hitbox_size;
    float speed;
    vec3 facing_direction;
    int score;
    bool move_direction[4];
    int animation_state;
};

struct Target {
    vec3 position;
    int hitbox_size;
};

struct Wall {
    vec3 position;
    int hitbox_size;
};

vec3 calcFacingDirection(vec3 mouse_position, struct Person* per);

void wallInit(struct Wall* wall, int wall_count, struct mat4* wall_matrix);

#endif
