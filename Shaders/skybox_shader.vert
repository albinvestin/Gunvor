#version 150

//uniform mat4 projectionMatrix;
//uniform mat4 total;
//uniform mat4 World_to_View;

uniform mat4 line;

in vec3 in_Position;

in vec3 InColor;
out	vec3 OutColor;

in vec2 inTexCoord;
out vec2 outTexCoord;

void main(void)
{
	OutColor = InColor;
	//gl_Position = projectionMatrix*World_to_View*total*vec4(in_Position, 1.0);
	gl_Position = line*vec4(in_Position, 1.0);
	outTexCoord = inTexCoord;
}
