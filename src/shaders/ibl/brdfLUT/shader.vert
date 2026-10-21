#version 450 core
layout (location = 0) in vec3 inPosition;
layout (location = 3) in vec2 texCoords;

layout (location = 0) out vec2 TexCoords;

void main()
{
	TexCoords = texCoords;
	gl_Position = vec4(inPosition, 1.0);
}
