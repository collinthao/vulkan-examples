#version 450 core

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 texCoords;
layout(location = 1) in vec4 FragPos;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 CameraPos;
layout(location = 4) in vec3 LightDir;
layout(location = 5) in vec4 LightSpace;

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform samplerCube depthTexture;

void main()
{
	vec3 lightDir = normalize(-LightDir);

	vec3 normal = normalize(-Normal);

	float diff = max(dot(normal, lightDir), 0.);
	
	vec3 diffuse = diff * vec3(1., 1., 1.);

	vec3 cameraDir = normalize(CameraPos - FragPos.xyz);
	
	vec3 lightDist = FragPos.xyz - vec3(0);

	float depthDist = texture(depthTexture, lightDist).r;

	vec3 textureSample = vec3(texture(texSampler, vec2(texCoords.x, texCoords.y)));
	
	float dist = length(lightDist);

	float shadow = (dist <= depthDist + 0.15) ? 1.0 : 0.5;	

	vec3 result = shadow * textureSample;
	FragColor = vec4(result, 1.);
}
