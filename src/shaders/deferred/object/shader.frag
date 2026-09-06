#version 450 core

layout(location = 0) out vec4 PositionColor;
layout(location = 1) out vec4 NormalColor;
layout(location = 2) out vec4 AlbedoColor;
layout(location = 3) out vec4 SpecularColor;

layout(location = 0) in vec2 texCoords;
layout(location = 1) in vec3 FragPos;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 CameraPos;
layout(location = 4) in vec3 LightPos[5];
layout(location = 9) in vec3 LightColor[5];

layout(binding = 1) uniform sampler2D texSampler;

void main()
{
	vec3 normal = normalize(Normal);

	vec3 color = vec3(texture(texSampler, vec2(texCoords.x , -texCoords.y)));

	PositionColor = vec4(FragPos, 1.);
	NormalColor = vec4(normal, 1.);
	AlbedoColor = vec4(color, 1.);
	SpecularColor = vec4(color, 1.);
}
