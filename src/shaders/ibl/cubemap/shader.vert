#version 450 core

layout(location = 0) in vec3 inPosition;

layout(binding=0) uniform  UniformData
{
	mat4 model;
	mat4 view;
	mat4 proj;
} ud;

layout(location = 0) out vec3 outPosition;

void main()
{
	outPosition = inPosition;	
	vec4 pos = ud.proj * ud.view * vec4(inPosition, 1.);
//	gl_Position = pos;
	gl_Position = pos.xyww;
}
