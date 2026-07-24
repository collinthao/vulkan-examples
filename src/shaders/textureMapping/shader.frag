#version 450 core
layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 texCoords;

layout(binding = 1) uniform sampler2D texSampler;

void main()
{
	FragColor = vec4(vec3(texture(texSampler, vec2(texCoords.x, -texCoords.y))), 1.);
}
