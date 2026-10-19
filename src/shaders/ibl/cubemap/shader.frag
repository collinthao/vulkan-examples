#version 450 core

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec3 inPosition;

layout(binding = 1) uniform samplerCube cubemap;

void main()
{
	vec3 cubemapTexture = texture(cubemap, inPosition).rgb;
//	cubemapTexture = cubemapTexture/ (cubemapTexture + vec3(1.0));
//	cubemapTexture = pow(cubemapTexture, vec3(1.0/2.2));
	FragColor = vec4(cubemapTexture, 1.);
}
