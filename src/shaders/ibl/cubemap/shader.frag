#version 450 core

layout(location = 0) out vec4 PositionColor;
layout(location = 1) out vec4 NormalColor;
layout(location = 2) out vec4 AlbedoColor;
layout(location = 3) out vec4 RoughnessAndMetallic;

layout(location = 0) in vec3 inPosition;

layout(binding = 1) uniform samplerCube cubemap;

void main()
{
	vec4 cubemapTexture = texture(cubemap, inPosition);
	PositionColor = vec4(vec3(cubemapTexture), 1.);
	NormalColor = vec4(1.);
	AlbedoColor = vec4(vec3(cubemapTexture),0.);
	RoughnessAndMetallic = vec4(1.);
}
