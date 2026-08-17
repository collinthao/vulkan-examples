#version 450 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;

layout(binding = 0) uniform OffscreenUniform
{
	mat4 model;
	mat4 projection;
	mat4 transforms[6];
	vec3 lightPos;
} ud;

void main()
{
	gl_Position = ud.model * vec4(inPosition, 1.);
}
