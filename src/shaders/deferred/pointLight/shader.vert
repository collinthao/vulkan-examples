#version 450 

layout(binding = 0) uniform UniformData
{	
	mat4 model;
	mat4 view;
	mat4 proj;
	vec3 lightColor;
} ud;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 LightColor;

void main()
{
	gl_Position = ud.proj* ud.view * ud.model * vec4(inPosition, 1.0);	
	LightColor = ud.lightColor;
}
