#version 450 core

layout(location = 0) in vec3 inPosition;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec2 texCoords;

layout(binding=0) uniform  UniformData
{
	mat4 model;
	mat4 view;
	mat4 proj;
} ud;

void main()
{
	gl_Position = ud.proj * ud.view * ud.model * vec4(inPosition, 1.);
	texCoords = inTexCoord;
}
