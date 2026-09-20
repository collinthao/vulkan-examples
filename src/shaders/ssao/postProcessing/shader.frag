#version 450

layout (location = 0) out vec4 fragColor;
layout (location = 0) in vec2 texCoords ;

layout (binding = 0) uniform sampler2D position;
layout (binding = 1) uniform sampler2D normal;
layout (binding = 2) uniform sampler2D albedo;
layout (binding = 3) readonly buffer DeferredUniform
{
	vec4 kernelSamples[64];	
	vec4 cameraPos;
} du;

void main()
{
	vec3 FragPos = texture(position, texCoords).rgb;	
	vec3 Normal = texture(normal, texCoords).rgb;	
	vec3 Albedo = texture(albedo, texCoords).rgb;	
	float Specular = 0.117;
	float power = 24.;

	vec3 lighting = Albedo * 0.1;	
	vec3 viewDir = normalize(du.cameraPos.xyz - FragPos);

	for (int i = 0; i < 5; i++)
	{
		float distance = length(du.lightPos[i].xyz - FragPos);
		if (distance < du.radius[i])
		{
			vec3 lightDir = normalize(du.lightPos[i].xyz - FragPos);
			vec3 diffuse = max(dot(Normal,lightDir), 0.0) * Albedo * du.lightColor[i].xyz;
			lighting += diffuse;
		};
	};

	fragColor = vec4(lighting, 1.);
}
