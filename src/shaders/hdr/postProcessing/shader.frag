#version 450

layout (location = 0) out vec4 fragColor;
layout (location = 0) in vec2 texCoords ;

layout (binding = 0) uniform sampler2D texture_sampler;

const float offset = 1./300.;

void main()
{
	float gamma = 2.2;

	vec3 hdrColor = texture(texture_sampler, texCoords).rgb;

	vec3 mapped = vec3(1.0) - exp(-hdrColor * 1.);

	mapped = pow(mapped, vec3(1./gamma));	

	fragColor = vec4(mapped, 1.);
//	fragColor = vec4(hdrColor, 1.);
}
