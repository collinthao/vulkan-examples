#version 450 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 3) in vec2 inTexCoords;

layout(location = 0) out vec2 texCoords;

void main()
{
	gl_Position = vec4(inPosition.x, inPosition.y, inPosition.z + 1., 1.);
	texCoords = inTexCoords;
}
