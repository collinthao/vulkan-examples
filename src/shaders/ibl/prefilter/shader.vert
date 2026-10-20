#version 450 core

layout(location = 0) in vec3 inPosition;

layout(binding = 0) uniform  HDRUniformData
{
	mat4 model;
	mat4 view;
	mat4 proj;
	float roughness;
} ud;

layout(location = 0) out vec3 outPosition;

void main()
{
	outPosition = inPosition;	
	gl_Position = ud.proj * ud.view * vec4(inPosition, 1.);
}
