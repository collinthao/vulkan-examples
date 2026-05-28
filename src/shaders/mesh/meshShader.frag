#version 450

#define MAX_POINT_LIGHTS 1 

struct SpotLight
{
	vec3 ambient; 
	vec3 diffuse; 
	vec3 specular; 
	vec3 position; 
	vec3 direction; 
	float cutOff;
	float outerCutOff;
};

struct DirectionalLight
{
	vec3 ambient; 
	vec3 diffuse; 
	vec3 specular; 
	vec3 direction; 
};

struct PointLight
{
	mat4 model;
	mat4 view;
	mat4 projection;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 position;
	vec3 color;
	float constant;
	float linear;
	float quadratic;
};

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout (location = 2) in vec3 Normal;
layout(location = 0) out vec4 outColor;
layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 3) uniform sampler2D shadowMap;

layout (binding = 2) uniform Lights
{
	PointLight pointLights[MAX_POINT_LIGHTS];
	DirectionalLight directionalLight;
	SpotLight spotLight;
} lights;

layout (location = 3) in vec3 CameraPos;
layout (location = 4) in vec3 FragPos;
layout (location = 5) in vec4 FragPosLightSpace;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 lightDir)
{
	vec3 projCoords = fragPosLightSpace.xyz/fragPosLightSpace.w;	
	projCoords = projCoords * 0.5 + 0.5;
	projCoords.y = 1.0 - projCoords.y;
	projCoords.x = 1.0 - projCoords.x;
	float closestDepth = texture(shadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;

	vec3 normal = normalize(Normal);

	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

	float shadow = 0.0;

	vec2 texelSize = 1.0/textureSize(shadowMap, 0);

	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x,y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}

	}

	shadow /= 9.0;

	if (projCoords.z > 1.0)
	{
		shadow = 0.0;
	}
	return shadow;
}


vec3 calculatePointLights(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDir);
	
vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir);
	
vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 viewDir);

void main()
{	
	vec3 norm = normalize(Normal);
	
	vec3 cameraDir = normalize(CameraPos - FragPos);

	vec3 result = calculateDirectionalLight(lights.directionalLight, norm, cameraDir);

	for (int i = 0; i < MAX_POINT_LIGHTS; i++)
	{
	//	result += calculatePointLights(lights.pointLights[i], norm, FragPos, cameraDir);	
	}

	//result += calculateSpotLight(lights.spotLight, norm, cameraDir);


	outColor = vec4(result, 1.);
}

vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction);

	float diff = max(dot(normal, lightDir), 0.);

	vec3 reflectDir = reflect(-lightDir, normal);

	float spec = pow(max(dot(reflectDir, viewDir), 0.), 32.f);

	vec3 ambient = light.ambient * vec3(texture(texSampler, fragTexCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(texSampler, fragTexCoord));
	vec3 specular = light.specular * spec * vec3(texture(texSampler, fragTexCoord)); //texSample for now

	float shadow = ShadowCalculation(FragPosLightSpace, lightDir);

	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * vec3(texture(texSampler, fragTexCoord));

	return lighting; 
}

vec3 calculatePointLights(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDir)
{
		vec3 lightDir = normalize(pointLight.position - fragPos);
		
		float diff = max(dot(normal, lightDir), 0.);
		
		vec3 reflectDir = reflect(lightDir, normal);

		float spec = pow(max(dot(viewDir, reflectDir), 0.f), 32.f);

		float lightDistance = length(pointLight.position - fragPos);
		float attenuation = 1.0/(pointLight.constant + pointLight.linear * lightDistance + pointLight.quadratic * (lightDistance));

		vec3 ambient = pointLight.ambient * vec3(texture(texSampler, fragTexCoord));
		vec3 diffuse = pointLight.diffuse * diff * vec3(texture(texSampler, fragTexCoord));
		//vec3 specular = pointLight.specular * spec * vec3(texture(specularTexture, fragTexCoord));

		ambient *= attenuation;
		diffuse *= attenuation;
		//specular *= attenuation;
	
		vec3 finalResult = ambient + diffuse;

		return finalResult + 0.5;
}

vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff)/epsilon, 0.0, 1.);

	float diff = max(dot(normal, lightDir), 0.);

	vec3 reflectDir = reflect(-lightDir, normal);

	float spec = pow(max(dot(halfwayDir, normal), 0.), 32.f);

	vec3 ambient = light.ambient * vec3(texture(texSampler, fragTexCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(texSampler, fragTexCoord));
//	vec3 specular = light.specular * spec * vec3(texture(specularTexture, fragTexCoord));

	ambient *= intensity;
        diffuse *= intensity;
 //       specular *= intensity;

	return ambient + diffuse;
}
