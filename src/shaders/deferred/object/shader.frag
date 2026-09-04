#version 450 core

layout(location = 0) out vec4 PositionColor;
layout(location = 1) out vec4 NormalColor;
layout(location = 2) out vec4 AlbedoColor;
layout(location = 3) out vec4 SpecularColor;

layout(location = 0) in vec2 texCoords;
layout(location = 1) in vec3 FragPos;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 CameraPos;
layout(location = 4) in vec3 LightPos[5];
layout(location = 9) in vec3 LightColor[5];

layout(binding = 1) uniform sampler2D texSampler;

const float constant = 1.f;
const float linear = 0.09f;
const float quadratic = 0.032;

void main()
{
	vec3 normal = normalize(Normal);


	vec3 color = vec3(texture(texSampler, vec2(texCoords.x * 10.f, texCoords.y * 10.f)));

	float ambientStrength = 0.0;	

	vec3 ambient = ambientStrength * color;
	vec3 lighting = vec3(0.0);

	for (int i = 0; i < 5; i++)
	{
		vec3 lightColor = LightColor[i];
		vec3 lightPos = LightPos[i];

		vec3 lightDir = normalize(lightPos - FragPos);
		
		float diff = max(dot(lightDir, normal), 0.);
		
		vec3 diffuse = lightColor * diff * color;

		vec3 result = diffuse;		

		// attenuation
		float distance = length(FragPos - lightPos);	
		result  *= 1.0/(distance * distance);	
		lighting += result;
	}
//	ambient *= attenuation;
//	specular *= attenuation;

	vec3 result = (ambient + lighting); 
	PositionColor = vec4(FragPos, 1.);
	NormalColor = vec4(normal, 1.);
	AlbedoColor = vec4(result, 1.);
	SpecularColor = vec4(result, 1.);
}
