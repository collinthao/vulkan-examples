#version 450 core

layout(location = 0) in vec3 inPosition;

layout(binding=0) uniform  HDRUniformData
{
	mat4 view[6];
	mat4 model;
	mat4 proj;
} ud;

layout(location = 0) out vec3 outPosition;

void main()
{
	outPosition = inPosition;	
	gl_Position = ud.model * vec4(inPosition, 1.);
}
