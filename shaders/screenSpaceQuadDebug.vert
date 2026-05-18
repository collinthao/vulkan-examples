#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec2 texCoords;

void main()
{
	//gl_Position =  vec4(inPosition * 2., 1.);
	gl_Position =  vec4(vec3(inPosition.x - 1., inPosition.y - 1., inPosition.z), 1.);
	texCoords = inTexCoord;
}
