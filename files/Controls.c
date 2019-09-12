#include <stdbool.h>
#include "Engine.h"
#include "../../common/VectorUtils3.h"
#include "Person.h" //Person includes Target

bool mouse_botton = {0};
vec3 mouse_location;

// Handle the key updates
void keyPress()
{
    player->move_direction[0] = glutKeyIsDown('w');
    player->move_direction[1] = glutKeyIsDown('d');
    player->move_direction[2] = glutKeyIsDown('s');
    player->move_direction[3] = glutKeyIsDown('a');
    calcFacingDirection(mouse_location, player); // Update player aim direction when moving
    if (glutKeyIsDown(GLUT_KEY_ESC))
    {
        glutExit();
    }
}

// Activate mouse buttons
void mouseClick(int button, int state, int x, int y)
{
    mouse_botton = (button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN);
}

// Handle the mouse-to-world projection and update players aim
void mouseDragged(int x, int y)
{
    // Need to project from viewport to world

    // Transform from 2d viewport coordinates to 3d normalized
    float xf = (2.0f * x) / width - 1.0f;
    float yf = 1.0f - (2.0f * y) / height; // inverse y so same as camera view
    float zf = 1.0f; // just initialize to something;
    vec3 ray_normalized = {xf, yf, zf};
    // Create a vec4 which has z to point forwards
    vec4 ray_clip = {ray_normalized.x, ray_normalized.y, -1.0, 1.0};

    // Inverse to camera coordinates
    vec4 ray_eye = MultVec4(InvertMat4(projectionMatrix), ray_clip);
    // Only need to un-project the x and y so set z,w to forward
    ray_eye.z = -1.0;
    ray_eye.w = 0.0;
    // Now to world
    vec3 ray_world = vec4tovec3(MultVec4(InvertMat4(lookat), ray_eye));
    ray_world = Normalize(ray_world);
    // Now we have a ray to compare to the world

    // Calculate the ray intersection with the ground
    vec3 ground_normal = {0,1,0}; // Flat ground
    vec3 cam_pos = cam_p;
    GLfloat distance = DotProduct(ground_normal, ScalarMult(cam_pos,-1.0)) / DotProduct(ground_normal, ray_world);
    mouse_location = VectorAdd(cam_pos, ScalarMult(ray_world, distance));
    mouse_location.y = 0;

    // If inverse aim, give the mouse location as oposite position on the ground
    if (inverse_aim)
    {
        mouse_location = SetVector(-mouse_location.x,0,-mouse_location.z);
    }

    // Calculate and update the player aim direction
    calcFacingDirection(mouse_location, player);
}
