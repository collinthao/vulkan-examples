#version 450 core

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 texCoords;
layout(location = 1) in vec3 FragPos;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 CameraPos;
layout(location = 4) in vec3 LightDir;
layout(location = 5) in vec4 LightSpace;

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler2D depthTexture;

const float constant = 1.0f;
const float linear = 0.09f;
const float quadratic = 0.032f;

// arbitrary light position since we're using directional light; could be uniform
const vec3 lightPos = vec3(1., 1., 2.);

float ShadowCalculation(vec4 fragPosLightSpace, vec3 lightDir)
{
	vec3 projCoords = fragPosLightSpace.xyz/fragPosLightSpace.w;	
	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture(depthTexture, projCoords.xy).r;
	float currentDepth = projCoords.z;

	vec3 normal = normalize(Normal);

	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	return shadow;
}

void main()
{
	float distance = length(lightPos - FragPos);
	float attenuation = 1.0/(constant - linear * distance + quadratic * (distance * distance));

	vec3 lightDir = normalize(-LightDir);

	vec3 normal = normalize(-Normal);

	float ambientStrength = 0.1;	

	vec3 ambient = ambientStrength * vec3(1., 1., 1.);

	float diff = max(dot(normal, lightDir), 0.);
	
	vec3 diffuse = diff * vec3(1., 1., 1.);

	vec3 cameraDir = normalize(CameraPos - FragPos);
	
	vec3 reflectDir = reflect(-lightDir, normal);

	float spec = pow(max(dot(cameraDir, reflectDir), 0.), 32.);	
	float specularStrength = .5;	

	vec3 specular = specularStrength * spec * vec3(1., 1., 1.);
	
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;
	
	float shadow = ShadowCalculation(LightSpace, lightDir);	
	vec3 textureSample = vec3(texture(texSampler, vec2(texCoords.x * 10., texCoords.y * 10.)));

	vec3 result = (ambient + (1.0 - shadow) + (diffuse + specular)) * textureSample;
	FragColor = vec4(result, 1.);
}
