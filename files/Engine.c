#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include "../../common/VectorUtils3.h"
#include "Engine.h"
extern vec3 cam_p;
extern vec3 cam_up;
extern vec3 cam_look;
extern mat4 lookat;
extern GLuint program;
extern GLuint shadow_shader;
extern GLuint target_shader;
extern bool inverse_aim;
extern bool follow_player;

// Boundaries for the players
int restriction[4] = {-50, -50, 50, 50};

int target_current_position = 0;
vec3 target_positions[8] = {
    {50, 0, 0},
    {-40, 0, 10},
    {25, 0, 10},
    {-10, 0, -25},
    {0, 0, -50},
    {-35, 0, -20},
    {35, 0, 20},
    {-40, 0, 25}
};

// Check Collision between ray and box
// Aim direction is always from origo, need to transform target
bool isCollisionRayCube(vec3 aim_dir, vec3 target_pos, int target_size)
{
    // Use axis aligned bounding box
    float tmin, tmax, tymin, tymax, tzmin, tzmax;

    // Check in 2 dimensions for two planes

    // Bounding box of target
    tmin = target_pos.x - (target_size / 2);
    tmax = target_pos.x + (target_size / 2);
    tzmin = target_pos.z - (target_size / 2);
    tzmax = target_pos.z + (target_size / 2);

    tmin = tmin * (1 / aim_dir.x);
    tmax = tmax * (1 / aim_dir.x);
    tzmin = tzmin * (1 / aim_dir.z);
    tzmax = tzmax * (1 / aim_dir.z);

    if (tmin > tmax)
    {
        // swap tmin tmax
        float temp = tmin;
        tmin = tmax;
        tmax = temp;
    }
    if (tzmin > tzmax)
    {
        // swap tymin tymax
        float temp = tzmin;
        tzmin = tzmax;
        tzmax = temp;
    }

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;

    // Done for one plane, prepare for next
    if (tzmin > tmin)
        tmin = tzmin;
    if (tzmax < tmax)
        tmax = tzmax;

    tymin = target_pos.y - (target_size / 2);
    tymax = target_pos.y + (target_size / 2);

    tymin = tymin * (1 / aim_dir.y);
    tymax = tymax * (1 / aim_dir.y);

    if (tymin > tymax)
    {
        // swap tymin tymax
        float temp = tymin;
        tymin = tymax;
        tymax = temp;
    }

    if ((tmin > tymax) || (tymin > tmax))
        return false;

    // Only to derive t value, could be used later.
    if (tymin > tmin)
        tmin = tymin;
    if (tymax < tmax)
        tmax = tymax;

    // Only intersection case left!
    return true;
}

// Check colllision between two cubes
bool isCollisionCubeCube(struct Person* player, struct Wall* object)
{
    // Use axis aligned bounding box
    float pxmin, pxmax, pzmin, pzmax;
    float wxmin, wxmax, wzmin, wzmax;
    vec3 p_pos = player->position;
    vec3 w_pos = object->position;

    int p_size = player->hitbox_size;
    int w_size = object->hitbox_size;

    pxmin = p_pos.x - (p_size / 2);
    pxmax = p_pos.x + (p_size / 2);
    pzmin = p_pos.z - (p_size / 2);
    pzmax = p_pos.z + (p_size / 2);

    wxmin = w_pos.x - (w_size / 2);
    wxmax = w_pos.x + (w_size / 2);
    wzmin = w_pos.z - (w_size / 2);
    wzmax = w_pos.z + (w_size / 2);

    return !(wxmin > pxmax || wxmax < pxmin || wzmax < pzmin || wzmin > pzmax);
}

// Move person according to the key pressed within the world restrictions and walls.
void movePerson(struct Person* player, struct Wall* object, int wall_count)
{

    vec3 pos = player->position;
    vec3 new_pos = {0,0,0};
    float speed = player->speed;
    bool direction[4];
    // direction[0] == up, [1] == right, [2] == down, [3] == left
    direction[0] = player->move_direction[0];
    direction[1] = player->move_direction[1];
    direction[2] = player->move_direction[2];
    direction[3] = player->move_direction[3];

    if (direction[0])
    {
        // Upwards is in isometric view forward in both x and z coordinates.
        new_pos.x -= speed;
        new_pos.z -= speed;
    }
    if (direction[1])
    {
        // Right is in isometric view forward in x and back z coordinates.
        new_pos.x += speed;
        new_pos.z -= speed;
    }
    if (direction[2])
    {
        // Downwards is in isometric view back in both x and z coordinates.
        new_pos.x += speed;
        new_pos.z += speed;
    }
    if (direction[3])
    {
        // Left is in isometric view back in x and forward in z coordinates.
        new_pos.x -= speed;
        new_pos.z += speed;
    }
    new_pos = VectorAdd(pos, new_pos);
    if (new_pos.x < restriction[0])
    {
        new_pos.x = restriction[0];
    }
    if (new_pos.z < restriction[1])
    {
        new_pos.z = restriction[1];
    }
    if (new_pos.x > restriction[2])
    {
        new_pos.x = restriction[2];
    }
    if (new_pos.z > restriction[3])
    {
        new_pos.z = restriction[3];
    }

    // To keep the momentum of the player when hitting a wall,
    // Check the new position if ok update and return.
    // If not ok, check first in x- and then in z-direction and update if ok in either.
    bool new_ok = {1};
    player->position = new_pos;
    int i;
    for (i = 0; i < wall_count; i++)
    {
        if (isCollisionCubeCube(player, &object[i]))
        {
            new_ok = 0;
        }
    }
    if (new_ok)
    {
        return;
    }

    // Divide update into two steps, one in x and one in z.
    bool x_ok = {1};
    bool z_ok = {1};

    // Check x update
    player->position = SetVector(new_pos.x, new_pos.y, pos.z);
    for (i = 0; i < wall_count; i++)
    {
        if (isCollisionCubeCube(player, &object[i]))
        {
            x_ok = 0;
        }
    }
    if (x_ok)
    {
        return;
    }

    player->position = SetVector(pos.x, new_pos.y, new_pos.z);
    for (i = 0; i < wall_count; i++)
    {
        if (isCollisionCubeCube(player, &object[i]))
        {
            z_ok = 0;
        }
    }
    if (z_ok)
    {
        return;
    }

    // Else
    player->position = pos;

}

// Handle shooting sequence, calculate if hitting target and if walls are
// in between, increases score and introduces game changes each 5th point.
void Shoot(struct Person* per, struct Target* tar, struct Wall* object, int wall_count)
{
    vec3 aim_dir = per->facing_direction;
    vec3 target_pos = tar->position;
    int target_size = tar->hitbox_size;
    vec3 player_pos = per->position;

    // Collision check uses aim direction from origo. need to transform target pos
    vec3 transformed_target = VectorSub(target_pos, player_pos);

    // Collision Ray Cube intersects in both positive and negative direction
    // Check first if target is behind player
    if (aim_dir.x >= 0 && transformed_target.x < 0)
    {
        return;
    }
    if (aim_dir.z >= 0 && transformed_target.z < 0)
    {
        return;
    }
    if (aim_dir.x <= 0 && transformed_target.x > 0)
    {
        return;
    }
    if (aim_dir.z <= 0 && transformed_target.z > 0)
    {
        return;
    }

    // Check if hitting target
    bool hit_target = isCollisionRayCube(aim_dir, transformed_target, target_size);

    if (hit_target)
    {
        // Calculate if wall is in between

        // Check which walls are hit
        bool hitted_wall[wall_count];
        int hbs_wall = object[0].hitbox_size;
        int i;
        for (i = 0; i < wall_count; i++)
        {
            // Need to transform wall accoring to the ray-cube intersection.
            vec3 transformed_wall = VectorSub(object[i].position, player_pos);
            // Check if wall is behind
            if (aim_dir.x >= 0 && transformed_wall.x + (hbs_wall / 2) < 0)
            {
                hitted_wall[i] = false;
            }
            else if (aim_dir.z >= 0 && transformed_wall.z + (hbs_wall / 2) < 0)
            {
                hitted_wall[i] = false;
            }
            else if (aim_dir.x <= 0 && transformed_wall.x - (hbs_wall / 2) > 0)
            {
                hitted_wall[i] = false;
            }
            else if (aim_dir.z <= 0 && transformed_wall.z - (hbs_wall / 2) > 0)
            {
                hitted_wall[i] = false;
            }
            else // wall is in same direction as aim dir
            {
                hitted_wall[i] = isCollisionRayCube(aim_dir, transformed_wall, hbs_wall);
            }
        }
        // Check if hitted walls are closer than target
        float distance_target = sqrt(pow(player_pos.x - target_pos.x, 2) +
                                    pow(player_pos.z - target_pos.z, 2));
        for (i = 0; i < wall_count; i++)
        {
            if (hitted_wall[i])
            {
                float distance_wall = sqrt(pow(player_pos.x - (object[i].position).x, 2) +
                                        pow(player_pos.z - (object[i].position).z, 2));
                if (distance_wall < distance_target)
                {
                    // Wall in between, end
                    return;
                }
            }
        }
        // No wall hitted and taget is shot
        per->score++;

        if (per->score == 5) { // mirror
            cam_p = SetVector(-80, 120, -80);
            lookat = lookAtv(cam_p, cam_look, cam_up);
        }
        else if (per->score == 10) { // inverse aim
            cam_p = SetVector(80, 120, 80); // Original
            lookat = lookAtv(cam_p, cam_look, cam_up);
            inverse_aim = 1;
        }
        else if (per->score == 15) { // Top down view
            inverse_aim = 0;
            cam_p = SetVector(1, 200, 0);
            lookat = lookAtv(cam_p, cam_look, cam_up);
        }
        else if (per->score == 20) { // Upside down
            cam_p = SetVector(80, 120, 80); // Original
            cam_up = SetVector(0,-1,0);
            lookat = lookAtv(cam_p, cam_look, cam_up);
        }
        else if (per->score == 25) { // Mirror inverse aim
            cam_up = SetVector(0,1,0);
            cam_p = SetVector(-80, 120, -80);
            lookat = lookAtv(cam_p, cam_look, cam_up);
            inverse_aim = 1;
        }
        else if (per->score == 30) { // Mirror Upside down
            cam_up = SetVector(0,-1,0);
            lookat = lookAtv(cam_p, cam_look, cam_up);
            inverse_aim = 0;
        }
        else if (per->score == 35) { // Top down inverse aim
            cam_up = SetVector(0,1,0);
            cam_p = SetVector(1, 200, 0);
            lookat = lookAtv(cam_p, cam_look, cam_up);
            inverse_aim = 1;
        }
        else if (per->score == 40) { // Follow player top down
            follow_player = true;
            inverse_aim = 0;
        }
        else if (per->score == 45) { // Top down mirror inverse aim
            follow_player = false;
            inverse_aim = 1;
            cam_p = SetVector(-1, 200, -1);
            cam_look = SetVector(0,0,0);
            lookat = lookAtv(cam_p, cam_look, cam_up);
        }
        else if (per->score == 50) { // Normal
            inverse_aim = 0;
            cam_p = SetVector(80, 120, 80);
            lookat = lookAtv(cam_p, cam_look, cam_up);
        }
        else if (per->score == 55) { // Follow player top down
            follow_player = true;
            inverse_aim = 1;
        }
        else if (per->score == 60) { // Winner?
            inverse_aim = 0;
            follow_player = false;
            cam_p = SetVector(80, 120, 80);
            cam_look = SetVector(0,0,0);
            lookat = lookAtv(cam_p, cam_look, cam_up);
        }
        else if (per->score > 60)
        {
            per->score = 60;
        }

        // Update the world to view matrix if new game change
        if ((per->score % 5) == 0) {
            glUseProgram(program);
            glUniformMatrix4fv(glGetUniformLocation(program, "World_to_View"), 1, GL_TRUE, lookat.m);
            glUseProgram(target_shader);
            glUniformMatrix4fv(glGetUniformLocation(target_shader, "World_to_View"), 1, GL_TRUE, lookat.m);
            glUseProgram(shadow_shader);
            glUniformMatrix4fv(glGetUniformLocation(shadow_shader, "World_to_View"), 1, GL_TRUE, lookat.m);
        }

        // Update the target position
        tar->position = target_positions[target_current_position];
        target_current_position++;
        if (target_current_position == 8)
        {
            target_current_position = 0;
        }
    }
}
