#version 450 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

layout(binding=0) uniform  UniformData
{
	mat4 model;
	mat4 view;
	mat4 proj;
} ud;

layout(location=0) out vec4 outColor;

void main()
{
	gl_Position = ud.proj * ud.view * ud.model * vec4(inPosition ,1.);
	gl_PointSize = 14.;
	outColor = inColor;
}
