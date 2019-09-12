#ifndef ENGINE_H
#define ENGINE_H

#include <stdbool.h>
// Engine handles Collision and physics
#include "Person.h"

bool isCollisionRayCube(vec3 aim_dir, vec3 target_pos, int target_size);

void movePerson(struct Person* player, struct Wall* object, int wall_count);

void Shoot(struct Person* per, struct Target* tar, struct Wall* object, int wall_count);


#endif
