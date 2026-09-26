#version 450 core

layout(location = 0) out vec4 PositionColor;
layout(location = 1) out vec4 NormalColor;
layout(location = 2) out vec4 AlbedoColor;

layout(location = 0) in vec2 texCoords;
layout(location = 1) in vec3 FragPos;
layout(location = 2) in vec3 Normal;

//layout(binding = 1) uniform sampler2D texSampler;

void main()
{
	vec3 fragPos = FragPos;
	vec3 normal = normalize(Normal);
	vec3 color = vec3(1.,vec2(0.));

	PositionColor = vec4(fragPos, 1.);
	NormalColor = vec4(normal, 1.);
	AlbedoColor = vec4(color, 1.);
}
