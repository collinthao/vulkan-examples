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
	vec3 lightColor = vec3(200.);
	vec3 color = vec3(texture(texSampler, vec2(texCoords.x * 10.f, texCoords.y * 10.f)));

	vec3 lightPos = LightPos;

	vec3 normal = normalize(Normal);

	// attenuation
	float distance = length(FragPos - lightPos);	
	float attenuation = 1.0/(distance * distance);	

	float ambientStrength = 0.0;	

	vec3 ambient = ambientStrength * color;

	vec3 lightDir = normalize(lightPos - FragPos);
	
	float diff = max(dot(lightDir, normal), 0.);
	
	vec3 diffuse = lightColor * diff * color;

//	ambient *= attenuation;
	diffuse *= attenuation;
//	specular *= attenuation;

	vec3 result = (ambient + diffuse); 
	FragColor = vec4(result, 1.);
}
