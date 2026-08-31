#version 450

layout (location = 0) out vec4 fragColor;
layout (location = 0) in vec2 texCoords ;

layout (binding = 0) uniform sampler2D bloom;
layout (binding = 1) uniform sampler2D scene;

void main()
{
	float gamma = 2.2;
	float exposure = 1.;

	vec3 hdrColor = texture(scene, texCoords).rgb;
	vec3 bloomColor = texture(bloom, texCoords).rgb;
	hdrColor += bloomColor;

	vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);

	mapped = pow(mapped, vec3(1./gamma));	

	fragColor = vec4(mapped, 1.);
}
