#version 450 core

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 texCoords;
layout(location = 1) in vec3 FragPos;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 CameraPos;
layout(location = 4) in vec3 LightPos;

layout(binding = 1) uniform sampler2D texSampler;

const float constant = 1.f;
const float linear = 0.09f;
const float quadratic = 0.032;

void main()
{


	vec3 color = vec3(1., 0.5, 0.31);
	vec3 lightPos = LightPos;

	vec3 normal = normalize(Normal);

	// attenuation
	float distance = length(lightPos - FragPos);	
	float attenuation = 1.0/(constant + linear * distance + quadratic* (distance * distance));	

	float ambientStrength = 0.1;	

	vec3 ambient = ambientStrength * color;

	vec3 lightDir = normalize(lightPos - FragPos);
	
	float diff = max(dot(normal, lightDir), 0.);
	
	vec3 diffuse = diff * color;

	vec3 cameraDir = normalize(CameraPos - FragPos);
	
	vec3 reflectDir = reflect(-lightDir, normal);

	float spec = pow(max(dot(cameraDir, reflectDir), 0.), 64.);	
	float specularStrength = .5;	

	vec3 specular = specularStrength * spec * color;

//	vec3 result = (ambient + diffuse + specular) * vec3(texture(texSampler, vec2(texCoords.x, texCoords.y)));

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	vec3 result = (ambient + diffuse + specular);

	FragColor = vec4(result, 1.);
}
