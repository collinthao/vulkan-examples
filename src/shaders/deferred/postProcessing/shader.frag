#version 450

layout (location = 0) out vec4 fragColor;
layout (location = 0) in vec2 texCoords ;

layout (binding = 0) uniform sampler2D position;
layout (binding = 1) uniform sampler2D normal;
layout (binding = 2) uniform sampler2D albedo;
layout (binding = 3) uniform DeferredUniform
{
	vec3 lightColor[5];
	vec3 lightPos[5];
	vec3 cameraPos;
} du;

void main()
{
	vec3 FragPos = texture(position, texCoords).rgb;	
	vec3 Normal = texture(normal, texCoords).rgb;	
	vec3 Albedo = texture(albedo, texCoords).rgb;	
	float Specular = 0.117;
	float power = 24.;

	vec3 lighting = Albedo * 0.1;	
	vec3 viewDir = normalize(du.cameraPos - FragPos);

	for (int i = 0; i < 5; i++)
	{
		vec3 lightDir = normalize(du.lightPos[i] - FragPos);
		vec3 diffuse = max(dot(Normal,lightDir), 0.0) * Albedo * du.lightColor[i];
		lighting += diffuse;
	};

	fragColor = vec4(lighting, 1.);
	//fragColor = vec4(1.);
}
