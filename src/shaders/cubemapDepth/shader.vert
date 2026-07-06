#version 450

layout(binding = 1) uniform UniformBufferObjectModel
{
mat4 model;
mat4 view;
mat4 proj;
mat4 lightSpaceMatrix;
vec3 fragColor;
vec3 cameraPos;
float deltaTime;
} ubom;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec3 pos;
layout(location = 5) in vec3 inScale;
layout(location = 6) in float inRot;
layout(location = 7) in int id;

layout(location = 0) out vec3 position;
layout(location = 1) flat out int type;

void main()
{
	vec4 posActual = ubom.proj * ubom.view * ubom.model * vec4((inPosition * 0.5) + pos, 1.);
	gl_Position = posActual;
	position = inPosition;
	type = id;
}
