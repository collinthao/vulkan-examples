#version 450 core

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 TexCoords;

layout(binding = 2) uniform sampler2D texSampler;

void main()
{
	vec3 depth = vec3(texture(texSampler, TexCoords));
	FragColor = vec4(vec3(depth.r), 1.);	
}
