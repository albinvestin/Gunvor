#include "../../common/VectorUtils3.h"
#include "../../common/GL_utilities.h"
#include <stdbool.h>
#include "DrawCalls.h"
extern gui_shader;
extern m_cube;

// Draw number using a scaled cube as a digital clock
void drawGUI(int number, float x_offset)
{
	float xpos = 0.45 + x_offset;
	float ypos = 0.55;

	if (number > 99)
	{
		xpos = 0.95 + x_offset;
		int remaining = number % 10;
		drawGUI((number-remaining) / 10, x_offset);
		number = remaining;
	}
	if (number > 9)
	{
		xpos = 0.70 + x_offset;
		int remaining = number % 10;
		drawGUI((number-remaining) / 10, x_offset);
		number = remaining;
	}

	bool index[7];
	/*
	bottom left = 0
	bottom = 1
	bottom right = 2
	middle = 3
	top left = 4
	top right = 5
	top = 6
	*/

	if (number == 0)
	{
		index[0] = 1;
		index[1] = 1;
		index[2] = 1;
		index[3] = 0;
		index[4] = 1;
		index[5] = 1;
		index[6] = 1;
	}
	if (number == 1)
	{
		index[0] = 0;
		index[1] = 0;
		index[2] = 1;
		index[3] = 0;
		index[4] = 0;
		index[5] = 1;
		index[6] = 1;
	}
	if (number == 2)
	{
		index[0] = 1;
		index[1] = 1;
		index[2] = 0;
		index[3] = 1;
		index[4] = 0;
		index[5] = 1;
		index[6] = 1;
	}
	if (number == 3)
	{
		index[0] = 0;
		index[1] = 1;
		index[2] = 1;
		index[3] = 1;
		index[4] = 0;
		index[5] = 1;
		index[6] = 1;
	}
	if (number == 4)
	{
		index[0] = 0;
		index[1] = 0;
		index[2] = 1;
		index[3] = 1;
		index[4] = 1;
		index[5] = 1;
		index[6] = 0;
	}
	if (number == 5)
	{
		index[0] = 0;
		index[1] = 1;
		index[2] = 1;
		index[3] = 1;
		index[4] = 1;
		index[5] = 0;
		index[6] = 1;
	}
	if (number == 6)
	{
		index[0] = 1;
		index[1] = 1;
		index[2] = 1;
		index[3] = 1;
		index[4] = 1;
		index[5] = 0;
		index[6] = 1;
	}
	if (number == 7)
	{
		index[0] = 0;
		index[1] = 0;
		index[2] = 1;
		index[3] = 0;
		index[4] = 1;
		index[5] = 1;
		index[6] = 1;
	}
	if (number == 8)
	{
		index[0] = 1;
		index[1] = 1;
		index[2] = 1;
		index[3] = 1;
		index[4] = 1;
		index[5] = 1;
		index[6] = 1;
	}
	if (number == 9)
	{
		index[0] = 0;
		index[1] = 1;
		index[2] = 1;
		index[3] = 1;
		index[4] = 1;
		index[5] = 1;
		index[6] = 1;
	}

	// Determined which sides should be drawn. Now scale and transpose cube accordingly
	mat4 line_matrix;
	if (index[0] == 1)
	{
		line_matrix = Mult(T(xpos,ypos,0),S(0.05, 0.25, 0.25));
		glUniformMatrix4fv(glGetUniformLocation(gui_shader, "line"), 1, GL_TRUE, line_matrix.m);
		DrawModel(m_cube, gui_shader, "in_Position", "in_Normal", "inTexCoord");
	}
	if (index[1] == 1)
	{
		line_matrix = Mult(T(xpos+0.05,ypos-0.10,0),S(0.05, 0.05, 0.25));
		glUniformMatrix4fv(glGetUniformLocation(gui_shader, "line"), 1, GL_TRUE, line_matrix.m);
		DrawModel(m_cube, gui_shader, "in_Position", "in_Normal", "inTexCoord");
	}
	if (index[2] == 1)
	{
		line_matrix = Mult(T(xpos+0.10,ypos,0),S(0.05, 0.25, 0.25));
		glUniformMatrix4fv(glGetUniformLocation(gui_shader, "line"), 1, GL_TRUE, line_matrix.m);
		DrawModel(m_cube, gui_shader, "in_Position", "in_Normal", "inTexCoord");
	}
	if (index[3] == 1)
	{
		line_matrix = Mult(T(xpos+0.05,ypos+0.10,0),S(0.05, 0.05, 0.25));
		glUniformMatrix4fv(glGetUniformLocation(gui_shader, "line"), 1, GL_TRUE, line_matrix.m);
		DrawModel(m_cube, gui_shader, "in_Position", "in_Normal", "inTexCoord");
	}
	if (index[4] == 1)
	{
		line_matrix = Mult(T(xpos,ypos+0.20,0),S(0.05, 0.25, 0.25));
		glUniformMatrix4fv(glGetUniformLocation(gui_shader, "line"), 1, GL_TRUE, line_matrix.m);
		DrawModel(m_cube, gui_shader, "in_Position", "in_Normal", "inTexCoord");
	}
	if (index[5] == 1)
	{
		line_matrix = Mult(T(xpos+0.10,ypos+0.20,0),S(0.05, 0.25, 0.25));
		glUniformMatrix4fv(glGetUniformLocation(gui_shader, "line"), 1, GL_TRUE, line_matrix.m);
		DrawModel(m_cube, gui_shader, "in_Position", "in_Normal", "inTexCoord");
	}
	if (index[6] == 1)
	{
		line_matrix = Mult(T(xpos+0.05,ypos+0.30,0),S(0.05, 0.05, 0.25));
		glUniformMatrix4fv(glGetUniformLocation(gui_shader, "line"), 1, GL_TRUE, line_matrix.m);
		DrawModel(m_cube, gui_shader, "in_Position", "in_Normal", "inTexCoord");
	}
}
