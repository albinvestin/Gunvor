#include "Person.h"
#include "../../common/VectorUtils3.h"

// Update the person facing direction from the current position and the mouse position
vec3 calcFacingDirection(vec3 mouse_position, struct Person* per)
{
    vec3 result;
    // VectorSub(a, b) takes a - b
    result = Normalize(VectorSub(mouse_position, per->position));
    result.y = 0;
    per->facing_direction = result;
    return result;
}

// Initialize the wall positions, size and matrices
void wallInit(struct Wall* wall, int wall_count, struct mat4* wall_matrix)
{
    int i;
    int hbs = 10;
    vec3 pos[wall_count];

    pos[0] = SetVector(-20, 0, 20);
    pos[1] = SetVector(-20, 0, -20);
    pos[2] = SetVector(20, 0, 20);
    pos[3] = SetVector(20, 0, -20);
    pos[4] = SetVector(-40, 0, 40);
    pos[5] = SetVector(-40, 0, -40);
    pos[6] = SetVector(40, 0, 40);
    pos[7] = SetVector(40, 0, -40);
    pos[8] = SetVector(-35, 0, 0);
    pos[9] = SetVector(35, 0, 0);
    pos[10] = SetVector(0, 0, 35);
    pos[11] = SetVector(0, 0, -35);

    for (i = 0; i < wall_count; i++)
    {
        wall[i].position = pos[i];
    	wall[i].hitbox_size = hbs;
        wall_matrix[i] = Mult(T(pos[i].x,pos[i].y,pos[i].z), S(hbs,hbs,hbs));
    }
}
