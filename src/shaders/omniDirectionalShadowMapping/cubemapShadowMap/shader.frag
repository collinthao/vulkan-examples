#version 450 core

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 texCoords;
layout(location = 1) in vec4 FragPos;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 CameraPos;
layout(location = 4) in vec3 LightDir;
layout(location = 5) in vec4 LightSpace;
layout(location = 6) in vec3 LightPos;

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform samplerCube depthTexture;

float ShadowCalculation(vec3 fragPos)
{
	vec3 fragToLight = fragPos - LightPos;
	float closestDepth = texture(depthTexture, fragToLight).r;
	closestDepth *= 25.f;
	float currentDepth = length(fragToLight);
	
	float bias = 0.05;
	currentDepth -= bias;
	float shadow = currentDepth <= closestDepth ? 1.0 : 0.0;
	return shadow;
}

void main()
{	
	float ambientStrength = 0.1;	

	vec3 ambient = ambientStrength * vec3(1., 1., 1.);

	vec3 lightDist = FragPos.xyz - LightPos;
	lightDist *= -1.;
	vec3 lightDir = normalize((LightPos - FragPos.xyz));

	vec3 normal = normalize(-Normal);

	float diff = max(dot(normal, lightDir), 0.);
	
	vec3 diffuse = diff * vec3(1., 1., 1.);
	vec3 cameraDir = normalize(CameraPos - FragPos.xyz);
	
	vec3 textureSample = vec3(texture(texSampler, vec2(texCoords.x * 10., texCoords.y * 10.)));

	float shadow = ShadowCalculation(FragPos.xyz);
	
	vec3 result = ((ambient + (shadow)) * diffuse) * textureSample;
//	vec3 result = shadow * textureSample;
//	vec3 result = vec3(shadow);

	FragColor = vec4(result, 1.);
}
