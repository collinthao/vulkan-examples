#version 450 core

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec3 inPosition;

layout(binding = 1) uniform samplerCube cubemap;

void main()
{
	vec4 cubemapTexture = texture(cubemap, inPosition);
	FragColor = vec4(vec3(cubemapTexture), 1.);
}
