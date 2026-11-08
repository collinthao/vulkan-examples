#version 450 core

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 texCoords;

layout(binding = 1) uniform sampler2D textureMap;

void main()
{
	vec3 albedo = vec3(texture(textureMap, vec2(texCoords.x, -texCoords.y)));
	FragColor = vec4(albedo, 1.);
}
