#version 450 core

layout(location = 0) in vec3 inPosition;

layout(binding=0) uniform  UniformData
{
	mat4 model;
	mat4 view;
	mat4 proj;
} ud;

void main()
{
	gl_Position = ud.proj * ud.view * ud.model * vec4(inPosition, 1.);
}
