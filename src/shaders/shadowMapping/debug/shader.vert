#version 450 core

layout(location = 0) in vec3 inPosition;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec2 TexCoords;

void main()
{
	gl_Position = vec4(vec3(inPosition.x * 0.75 - 0.75, inPosition.y * 0.75 - 0.75, inPosition.z), 1.);

	TexCoords = inTexCoord;
}
