#version 450

layout (location = 0) out vec4 fragColor;
layout (location = 0) in vec2 texCoords ;

layout (binding = 0) uniform sampler2D texture_sampler;

float near = 0.1f;
float far = 100.f;

float LinearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0;
	return (2. * near * far)/(far + near - z * (far - near));
}

void main()
{
    float depth = LinearizeDepth(texture(texture_sampler, texCoords).r)/far;
    fragColor = vec4(vec3(depth), 1.); 
}
