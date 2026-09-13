#version 450 core

layout(location = 0) in vec3 inPosition;

layout(binding=0) uniform  UniformData
{
	mat4 model;
	mat4 view;
	mat4 proj;
} ud;

layout(location=0) out mat4 Model;
layout(location=4) out mat4 View;
layout(location=8) out mat4 Proj;

void main()
{
	gl_PointSize = 100;
	gl_Position = ud.proj * ud.view * ud.model * vec4(inPosition, 1.);
	Model = ud.model;
	View = ud.view;
	Proj = ud.proj;
}
