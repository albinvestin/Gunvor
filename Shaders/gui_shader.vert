#version 150

in  vec3 in_Position;

uniform mat4 line;


void main(void)
{
	gl_Position = line*vec4(in_Position, 1.0);
}
