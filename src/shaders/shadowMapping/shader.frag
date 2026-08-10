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

// arbitrary light position since we're using directional light; could be uniform
const vec3 lightPos = vec3(3.f, -6.f, 0.f);

float ShadowCalculation(vec4 projCoords, vec3 lightDir, vec2 offset)
{
	float shadow = 1.0;

	if (projCoords.z > -1.0 && projCoords.z < 1.0) 
	{
		float dist = texture(depthTexture, projCoords.st + offset).r;
		if ( projCoords.w > 0.0 && dist < projCoords.z ) 
		{
			shadow = 0.1;
		}
	}

	return shadow;
}

float filterPCF(vec4 sc)
{
	ivec2 texDim = textureSize(depthTexture, 0);
	float scale = 1.5;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += ShadowCalculation(sc, LightDir, vec2(dx*x, dy*y));
			count++;
		}
	
	}
	return shadowFactor / count;
}

void main()
{
	vec3 lightDir = normalize(-LightDir);

	vec3 normal = normalize(-Normal);

	float diff = max(dot(normal, lightDir), 0.);
	
	vec3 diffuse = diff * vec3(1., 1., 1.);

	vec3 cameraDir = normalize(CameraPos - FragPos);
	
	float shadow = filterPCF(LightSpace/LightSpace.w);	
	vec3 textureSample = vec3(texture(texSampler, vec2(texCoords.x * 10., texCoords.y * 10.)));

	vec3 result = (shadow) * textureSample;
	FragColor = vec4(result, 1.);
}
