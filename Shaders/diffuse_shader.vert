#version 150

uniform mat4 projectionMatrix;
uniform mat4 total;
uniform mat4 World_to_View;
mat3 normaltotal;
in  vec3 in_Position;
in vec3 InColor;
out	vec3 OutColor;
in vec3 in_Normal;
out vec3 out_Normal;
in vec2 inTexCoord;
out vec2 outTexCoord;
out vec3 position;

void main(void)
{
	normaltotal = mat3(total);
	OutColor = InColor;
	out_Normal = normaltotal*in_Normal;
	gl_Position = projectionMatrix*World_to_View*total*vec4(in_Position, 1.0);
	outTexCoord = inTexCoord;
}
