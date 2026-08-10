#version 450 core

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 TexCoords;

layout(binding = 2) uniform sampler2D texSampler;

float LinearizeDepth(float depth)
{
  float n = 0.1;
  float f = 10.f;
  float z = depth;
  return (2.0 * n) / (f + n - z * (f - n));	
}

void main()
{
	vec3 depth = vec3(texture(texSampler, TexCoords));

	FragColor = vec4(vec3(LinearizeDepth(depth.r)), 1.);	
}
