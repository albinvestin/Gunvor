#version 150

uniform mat4 projectionMatrix;
uniform mat4 total;
uniform mat4 World_to_View;
in  vec3 in_Position;

void main(void)
{
	gl_Position = projectionMatrix*World_to_View*total*vec4(in_Position, 1.0);
}
