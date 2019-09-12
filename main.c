// Lab 1-1.
// This is the same as the first simple example in the course book,
// but with a few error checks.
// Remember to copy your file to a new on appropriate places during the lab so you keep old results.
// Note that the files "lab1-1.frag", "lab1-1.vert" are required.

// Should work as is on Linux and Mac. MS Windows needs GLEW or glee.
// See separate Visual Studio version of my demos.
#ifdef __APPLE__
	#include <OpenGL/gl3.h>
	// Linking hint for Lightweight IDE
	// uses framework Cocoa
#endif
#include <stdio.h>
#include "../common/Linux/MicroGlut.h"
#include "../common/GL_utilities.h"
#include "../common/VectorUtils3.h"
#include <math.h>
#include "../common/loadobj.h"
#include "../common/LoadTGA.h"

int number = 0;

// Globals
#define width 800

#define height 800

#define near 1.0

#define far 500.0

#define right 0.5

#define left -0.5

#define top 0.5

#define bottom -0.5

#define PI 3.1415

// Game declarations
struct Person* player;
struct Target* target;
struct Wall* wall;

mat4 projectionMatrix =
{    2.0f*near/(right-left), 0.0f, (right+left)/(right-left), 0.0f,

	0.0f, 2.0f*near/(top-bottom), (top+bottom)/(top-bottom), 0.0f,

	0.0f, 0.0f, -(far + near)/(far - near), -2*far*near/(far - near),

	0.0f, 0.0f, -1.0f, 0.0f
};

vec3 cam_p = {80, 120, 80}; // Original
//vec3 cam_p = {0, 90, 1}; // For debugging
vec3 cam_look = {0, 0, 0};
vec3 cam_up = {0, 1, 0};

mat4 lookat;

Model *m_cube;
Model *m_white_foot;
Model *m_gun;
Model *m_background;

unsigned int vertexArrayObjID;
unsigned int vertexBufferObjID;
unsigned int IndexBufferObjID;
unsigned int NormalBufferObjID;

// Reference to shader program
GLuint program;
GLuint target_shader;
GLuint shadow_shader;
GLuint gui_shader;
GLuint skybox_shader;

// Textures
// GLuint myTex;
GLuint groundTex;
GLuint white_footTex;
GLuint gunTex;
GLuint backgroundTex;
GLuint float_cubeTex;
GLuint wallTex;

mat4 skybox_total;
mat4 ground_total;
//mat4 rot_time;

//mat4 model_rot, rot, trans, total;

mat4 player_matrix[3];
mat4 players_matrix;
mat4 gun_matrix;
mat4 guns_matrix;
mat4 target_matrix;
int wall_count = 12;
mat4 wall_matrix[12];
mat4 float_cube_matrix[3];
mat4 float_move_cube_matrix[3];
mat4 shot_trace_matrix;

// Game indicatiors
bool inverse_aim = 0;
bool follow_player = 0;
int game_time = 120;
bool update_animation = 0;

// Include game files
#include "files/Person.h"
#include "files/Engine.h"
#include "files/Controls.c" // since controls need access to player and lookat
#include "files/DrawCalls.h"

// Calculate Player matrices and animate feets
void playerAnimation()
{
	vec3 pos = player->position;
	vec3 aim_dir = player->facing_direction;
	int a_state = player->animation_state;
	int i;
	bool is_moving = 0;
	for (i = 0; i < 4; i++) {
		if (player->move_direction[i]) {
			is_moving = true;
		}
	}
	if (update_animation) {
		if (is_moving) {
			a_state++;
			if (a_state > 2) {
				a_state = 1;
			}
		}
		else {
			a_state = 0;
		}
		player->animation_state = a_state;
		update_animation = 0;
	}

	// Player matrix index 0 = body, 1 = left foot, 2 = right foot
	// body is a litte above ground
	player_matrix[0] = Mult(T(pos.x,pos.y+2,pos.z), S(0.015,0.015,0.015));
	player_matrix[1] = S(0.005,0.005,0.005);
	float angle = acos( DotProduct(SetVector(1,0,0), SetVector(aim_dir.x, 0, aim_dir.z)));
	if (aim_dir.z < 0)
	{
		player_matrix[1] = Mult(Ry(angle), player_matrix[1]);
	}
	else
	{
		player_matrix[1] = Mult(Ry(-angle), player_matrix[1]);
	}
	player_matrix[2] = player_matrix[1];
	vec3 left_vec = Normalize(CrossProduct(SetVector(0,1,0), aim_dir));
	left_vec = SetVector(3*left_vec.x,3*left_vec.y,3*left_vec.z);
	aim_dir = SetVector(3*aim_dir.x,3*aim_dir.y,3*aim_dir.z);
	if (a_state == 0) // Still
	{
		player_matrix[1] = Mult(T(pos.x+left_vec.x,pos.y,pos.z+left_vec.z), player_matrix[1]);
		player_matrix[2] = Mult(T(pos.x-left_vec.x,pos.y,pos.z-left_vec.z), player_matrix[2]);
	}
	else if (a_state == 1) // Left foot forward, right backward
	{
		player_matrix[1] = Mult(T(pos.x+left_vec.x+aim_dir.x,pos.y,pos.z+left_vec.z+aim_dir.z), player_matrix[1]);
		player_matrix[2] = Mult(T(pos.x-left_vec.x-aim_dir.x,pos.y,pos.z-left_vec.z-aim_dir.z), player_matrix[2]);

	}
	else if (a_state == 2) // Left foot backward, right forward
	{
		player_matrix[1] = Mult(T(pos.x+left_vec.x-aim_dir.x,pos.y,pos.z+left_vec.z-aim_dir.z), player_matrix[1]);
		player_matrix[2] = Mult(T(pos.x-left_vec.x+aim_dir.x,pos.y,pos.z-left_vec.z+aim_dir.z), player_matrix[2]);
	}

}

// Timers

// Update animations
void animatonTimer(int value)
{
	update_animation = 1;
	value++;
	glutTimerFunc(200, &animatonTimer, value);
}

// Count down the game time
void gameTime(int value)
{
	if (game_time > 0) {
		game_time--;
	}
	else
	{
		game_time = 0;
		// TODO: exit program?
	}
	glutTimerFunc(1000, &gameTime, value);
}

// Used for floating cubes
float float_cube_angle = 0;
float circle_radius = 120;
float circle_angle = 0;

// Game update
void OnTimer(int value)
{
	keyPress(); // Update the pressed keys
	movePerson(player, wall, wall_count); // move player according to pressed keys
	playerAnimation(); // Animate feets and update player matrices

	// Update Player shadow matrix
	vec3 pos = player->position;
	players_matrix = Mult(T(pos.x+3,1,pos.z-3), S(0.020,0,0.020));

	// Update Shot trace matrix
	float shot_lenght = Norm(VectorSub(mouse_location, pos)) - 14;
	shot_trace_matrix = Mult(T(14 + shot_lenght/2,0,0),S(shot_lenght, 0.1, 0.1));

	// Update Gun matrix
	vec3 face_dir = player->facing_direction;
	gun_matrix = Mult(S(0.5,0.5,0.5), Ry(PI/2));
	gun_matrix = Mult(T(3, 0, 0), gun_matrix);
	// Calculate the angle set to the gun and trace matrix
	float angle = acos( DotProduct(SetVector(1,0,0), SetVector(face_dir.x, 0, face_dir.z)));
	if (face_dir.z < 0)
	{
		gun_matrix = Mult(Ry(angle), gun_matrix);
		shot_trace_matrix = Mult(Ry(angle), shot_trace_matrix);
	}
	else
	{
		gun_matrix = Mult(Ry(-angle), gun_matrix);
		shot_trace_matrix = Mult(Ry(-angle), shot_trace_matrix);
	}
	gun_matrix = Mult(T(pos.x,pos.y,pos.z), gun_matrix);
	shot_trace_matrix = Mult(T(pos.x,pos.y,pos.z), shot_trace_matrix);
	// Update Gun Shadow matrix
	guns_matrix = Mult(S(1.020,0,1.020), gun_matrix);
	guns_matrix = Mult(T(3,1,-5), guns_matrix);

	// Update Target matrix
	vec3 tpos = target->position;
	target_matrix = Mult(T(tpos.x,tpos.y,tpos.z), S(5,5,5));

	// Check if shooting
	if (mouse_botton)
	{
		// Check if correct hit without interference of wall.
		// Increases score and give new target position if correct hit.
		Shoot(player, target, wall, wall_count);
	}

	// Update Stationary Floating cube matrices (3)
	float_cube_angle += 0.002*PI; // The rotational angle around its own shoulder
	if (float_cube_angle > 2*PI) {
		float_cube_angle = 0;
	}
	// First
	float_cube_matrix[0] = Mult(Ry(float_cube_angle), Rz(float_cube_angle));
	float_cube_matrix[0] = Mult(S(2,2,2), float_cube_matrix[0]);
	float_cube_matrix[0] = Mult(T(-60, 10, 40), float_cube_matrix[0]);
	// Second
	float_cube_matrix[1] = Mult(Ry(-float_cube_angle), Rx(float_cube_angle));
	float_cube_matrix[1] = Mult(S(3,1,3), float_cube_matrix[1]);
	float_cube_matrix[1] = Mult(T(-40, -70, 70), float_cube_matrix[1]);
	// Third
	float_cube_matrix[2] = Mult(Rx(-float_cube_angle), Rz(float_cube_angle));
	float_cube_matrix[2] = Mult(S(5,5,5), float_cube_matrix[2]);
	float_cube_matrix[2] = Mult(T(70, -70, -60), float_cube_matrix[2]);

	// Update Circular orbit Floating cube matrices (3)
	// Circular orbit
	circle_angle += 0.001*PI;
	if (circle_angle > 2*PI)
	{
		circle_angle = 0;
	}
	float circle_x = circle_radius * cos(circle_angle);
	float circle_y = circle_radius * sin(circle_angle);
	// First
	float_move_cube_matrix[0] = Ry(float_cube_angle);
	float_move_cube_matrix[0] = Mult(S(2,2,2), float_move_cube_matrix[0]);
	float_move_cube_matrix[0] = Mult(T(circle_x,-30, circle_y), float_move_cube_matrix[0]);
	// Second
	float_move_cube_matrix[1] = Rx(float_cube_angle);
	float_move_cube_matrix[1] = Mult(S(2,2,2), float_move_cube_matrix[1]);
	float_move_cube_matrix[1] = Mult(T(60+circle_x,-120-circle_y/3,60-circle_y), float_move_cube_matrix[1]);
	// Third
	circle_x = circle_radius * cos(circle_angle-0.5);
	circle_y = circle_radius * sin(circle_angle-0.5);
	float_move_cube_matrix[2] = Mult(Rx(float_cube_angle/2), Rz(-float_cube_angle));
	float_move_cube_matrix[2] = Mult(S(2,2,2), float_move_cube_matrix[2]);
	float_move_cube_matrix[2] = Mult(T(-60+circle_x*1.4,-120,-60+circle_y*1.3), float_move_cube_matrix[2]);

	// Timer value and refresh
	value++;
    glutPostRedisplay();

    glutTimerFunc(20, &OnTimer, value);
}

// Initialize
void init(void)
{
	// Initialize game structs

	// Init Player
	player = malloc(sizeof(struct Person));
	vec3 pos = {0,0,10}; // neg z to right, neg x is up.
	vec3 face_dir = {1,0,0};
	player->position = pos;
	player->hitbox_size = 6;
	player->speed = 1;
	player->facing_direction = face_dir;
	player->score = 0;

	// Init Target
	pos = SetVector(35, 0, 20);
	target = malloc(sizeof(struct Target));
	target->position = pos;
	target->hitbox_size = 5;

	// Init Walls
	wall = (struct Wall*) malloc(sizeof(struct Wall)*wall_count);
	wallInit(wall, wall_count, wall_matrix);

	// Init Ground
	ground_total = Mult(T(0,-60,0),S(120, 120, 120));

	// Load models
	m_background = LoadModelPlus("Models/skybox_back.obj");
	m_cube = LoadModelPlus("Models/cubeplus.obj");
	m_white_foot = LoadModelPlus("Models/white_foot.obj");
	m_gun = LoadModelPlus("Models/m4a1.obj");

	// Load textures
	LoadTGATextureSimple("Textures/grass.tga", &groundTex);
	LoadTGATextureSimple("Textures/drakeq_bk.tga", &backgroundTex);
	LoadTGATextureSimple("Textures/white_foot.tga", &white_footTex);
	LoadTGATextureSimple("Textures/rif_m4a1.tga", &gunTex);
	LoadTGATextureSimple("Textures/white_foot2.tga", &float_cubeTex);
	LoadTGATextureSimple("Textures/grass.tga", &wallTex);

	dumpInfo();

	// GL Inits
	glClearColor(0.38,0.54,0.78,0);
	glDisable(GL_DEPTH_TEST);
	printError("GL inits");

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Load and compile shader
	program = loadShaders("Shaders/diffuse_shader.vert", "Shaders/diffuse_shader.frag");
	target_shader = loadShaders("Shaders/target_shader.vert", "Shaders/target_shader.frag");
	shadow_shader = loadShaders("Shaders/shadow_shader.vert", "Shaders/shadow_shader.frag");
	gui_shader = loadShaders("Shaders/gui_shader.vert", "Shaders/gui_shader.frag");
	skybox_shader = loadShaders("Shaders/skybox_shader.vert", "Shaders/skybox_shader.frag");
	printError("init shader");

	//  Generate Frustum and load to shaders
	lookat = lookAtv(cam_p, cam_look, cam_up);

	glUseProgram(program);
	glUniformMatrix4fv(glGetUniformLocation(program, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniformMatrix4fv(glGetUniformLocation(program, "World_to_View"), 1, GL_TRUE, lookat.m);

	glUseProgram(target_shader);
	glUniformMatrix4fv(glGetUniformLocation(target_shader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniformMatrix4fv(glGetUniformLocation(target_shader, "World_to_View"), 1, GL_TRUE, lookat.m);

	glUseProgram(shadow_shader);
	glUniformMatrix4fv(glGetUniformLocation(shadow_shader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniformMatrix4fv(glGetUniformLocation(shadow_shader, "World_to_View"), 1, GL_TRUE, lookat.m);

	glUseProgram(skybox_shader);
	mat4 line_matrix;
	line_matrix = S(0.98,0.98,0.98); // Scale a litte smaller to give a nice boarder
	glUniformMatrix4fv(glGetUniformLocation(skybox_shader, "line"), 1, GL_TRUE, line_matrix.m);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, backgroundTex);

	printError("init arrays");

	// Z buffers
	glEnable(GL_DEPTH_TEST);
}

void display(void)
{
	//glCullFace(GL_BACK);
	//glEnable(GL_CULL_FACE);

	printError("pre display");
	glUseProgram(program);
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Check if camera following player
	if (follow_player)
	{
		cam_p = SetVector(player->position.x+1, 40, player->position.z+1);
		cam_look = player->position;
		lookat = lookAtv(cam_p, cam_look, cam_up);
		glUseProgram(program);
		glUniformMatrix4fv(glGetUniformLocation(program, "World_to_View"), 1, GL_TRUE, lookat.m);
		glUseProgram(target_shader);
		glUniformMatrix4fv(glGetUniformLocation(target_shader, "World_to_View"), 1, GL_TRUE, lookat.m);
		glUseProgram(shadow_shader);
		glUniformMatrix4fv(glGetUniformLocation(shadow_shader, "World_to_View"), 1, GL_TRUE, lookat.m);
	}

	//Disable Z-buffer and draw background
	glUseProgram(skybox_shader);
	glDisable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, backgroundTex);
	DrawModel(m_background, skybox_shader, "in_Position", "in_Normal", "inTexCoord");
	glEnable(GL_DEPTH_TEST); //Enable Z-buffer

	// Draw Stationary Floating cubes (3)
	glUseProgram(program);
	glUniformMatrix4fv(glGetUniformLocation(program, "total"), 1, GL_TRUE, float_cube_matrix[0].m);
	glBindTexture(GL_TEXTURE_2D, float_cubeTex);
	glActiveTexture(GL_TEXTURE0);
	DrawModel(m_cube, program, "in_Position", "in_Normal", "inTexCoord");
	glUniformMatrix4fv(glGetUniformLocation(program, "total"), 1, GL_TRUE, float_cube_matrix[1].m);
	DrawModel(m_cube, program, "in_Position", "in_Normal", "inTexCoord");
	glUniformMatrix4fv(glGetUniformLocation(program, "total"), 1, GL_TRUE, float_cube_matrix[2].m);
	DrawModel(m_cube, program, "in_Position", "in_Normal", "inTexCoord");

	// Draw Circular orbit Floating cube (3)
	glUniformMatrix4fv(glGetUniformLocation(program, "total"), 1, GL_TRUE, float_move_cube_matrix[0].m);
	DrawModel(m_cube, program, "in_Position", "in_Normal", "inTexCoord");
	glUniformMatrix4fv(glGetUniformLocation(program, "total"), 1, GL_TRUE, float_move_cube_matrix[1].m);
	DrawModel(m_cube, program, "in_Position", "in_Normal", "inTexCoord");
	glUniformMatrix4fv(glGetUniformLocation(program, "total"), 1, GL_TRUE, float_move_cube_matrix[2].m);
	DrawModel(m_cube, program, "in_Position", "in_Normal", "inTexCoord");

	// Draw Ground
	glUseProgram(program);
	glUniformMatrix4fv(glGetUniformLocation(program, "total"), 1, GL_TRUE, ground_total.m);
	glBindTexture(GL_TEXTURE_2D, groundTex);
	glActiveTexture(GL_TEXTURE0);
	DrawModel(m_cube, program, "in_Position", "in_Normal", "inTexCoord");

	// Draw Shadows
	// Draw Player shadow
	glUseProgram(shadow_shader);
	glUniformMatrix4fv(glGetUniformLocation(shadow_shader, "total"), 1, GL_TRUE, players_matrix.m);
	DrawModel(m_white_foot, shadow_shader, "in_Position", "in_Normal", "inTexCoord");
	// Draw Gun shadow
	glUniformMatrix4fv(glGetUniformLocation(shadow_shader, "total"), 1, GL_TRUE, guns_matrix.m);
	DrawModel(m_gun, shadow_shader, "in_Position", "in_Normal", "inTexCoord");

	// Draw Player
	glUseProgram(program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, white_footTex);
	glUniformMatrix4fv(glGetUniformLocation(program, "total"), 1, GL_TRUE, player_matrix[0].m);
	DrawModel(m_white_foot, program, "in_Position", "in_Normal", "inTexCoord");
	glUniformMatrix4fv(glGetUniformLocation(program, "total"), 1, GL_TRUE, player_matrix[1].m);
	DrawModel(m_white_foot, program, "in_Position", "in_Normal", "inTexCoord");
	glUniformMatrix4fv(glGetUniformLocation(program, "total"), 1, GL_TRUE, player_matrix[2].m);
	DrawModel(m_white_foot, program, "in_Position", "in_Normal", "inTexCoord");

	// Draw gun
	glBindTexture(GL_TEXTURE_2D, gunTex);
	glActiveTexture(GL_TEXTURE0);
	glUniformMatrix4fv(glGetUniformLocation(program, "total"), 1, GL_TRUE, gun_matrix.m);
	DrawModel(m_gun, program, "in_Position", "in_Normal", "inTexCoord");

	// Use Target shader
	// Draw Target
	glUseProgram(target_shader);
	glUniformMatrix4fv(glGetUniformLocation(target_shader, "total"), 1, GL_TRUE, target_matrix.m);
	DrawModel(m_cube, target_shader, "in_Position", "in_Normal", "inTexCoord");

	// Draw shot trace
	if (mouse_botton) {
		glUseProgram(target_shader);
		glUniformMatrix4fv(glGetUniformLocation(target_shader, "total"), 1, GL_TRUE, shot_trace_matrix.m);
		DrawModel(m_cube, target_shader, "in_Position", "in_Normal", "inTexCoord");
	}

	// Draw walls
	glUseProgram(program);
	glBindTexture(GL_TEXTURE_2D, wallTex);
	glActiveTexture(GL_TEXTURE0);
	int i;
	for (i = 0; i < wall_count; i++)
	{
		glUniformMatrix4fv(glGetUniformLocation(program, "total"), 1, GL_TRUE, wall_matrix[i].m);
		DrawModel(m_cube, program, "in_Position", "in_Normal", "inTexCoord");
	}

	// Disable Z-buffer and draw gui_shader
	glDisable(GL_DEPTH_TEST);
	glUseProgram(gui_shader);
	drawGUI(player->score, 0);
	drawGUI(game_time, -1.4f);
	glEnable(GL_DEPTH_TEST);

	printError("display");
	glutSwapBuffers();
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 2);
	glutInitWindowSize (width, height);
	glutInitDisplayMode(GLUT_DEPTH);
	glutCreateWindow ("Gunvor");
	glutDisplayFunc(display);
	glutPassiveMotionFunc(mouseDragged);
	glutMouseFunc(mouseClick);
	glutTimerFunc(20, &OnTimer, 0);
	glutTimerFunc(1000, &gameTime, 0);
	glutTimerFunc(200, &animatonTimer, 0);
	init();
	glutMainLoop();

	return 0;
}
