#version 450

layout (location = 0) out vec4 fragColor;
layout (location = 0) in vec2 texCoords ;

layout (binding = 0) uniform sampler2D position;
layout (binding = 1) uniform sampler2D normal;
layout (binding = 2) uniform sampler2D albedo;
layout (binding = 3) readonly buffer DeferredUniform
{
	vec4 cameraPos;
} du;

void main()
{
	vec3 FragPos = texture(position, texCoords).rgb;	
	vec3 Normal = texture(normal, texCoords).rgb;	
	vec3 Albedo = texture(albedo, texCoords).rgb;	
	float Specular = 0.117;
	float power = 24.;
	
	vec3 lightPos = vec3(2.);
	vec3 lightColor = vec3(1., 0., 0.);

	vec3 lighting = Albedo * 0.1;	
	vec3 viewDir = normalize(du.cameraPos.xyz - FragPos);

	float distance = length(lightPos - FragPos);

	vec3 lightDir = normalize(lightPos - FragPos);
	vec3 diffuse = max(dot(Normal,lightDir), 0.0) * Albedo * lightColor;

	lighting += diffuse;

	fragColor = vec4(lighting, 1.);
}
