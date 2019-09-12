#version 150

uniform mat4 World_to_View;

out vec4 out_Color;
in vec3 OutColor;
in vec3 out_Normal;
in vec2 outTexCoord;

uniform sampler2D texUnit;

const vec3 light = vec3(-0.87, 0.35, 0.35);

void main(void)
{
	float calculated_light = clamp(max(0,normalize(dot(out_Normal,light))) + 0.35, 0, 1);

	out_Color = calculated_light*texture(texUnit, outTexCoord);
}
