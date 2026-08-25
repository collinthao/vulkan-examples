#version 450 core

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 texCoords;
layout(location = 1) in vec3 FragPos;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 CameraPos;
layout(location = 4) in vec3 LightPos;

layout(binding = 1) uniform sampler2D texSampler;

void main()
{
	vec3 color = vec3(1., 0.5, 0.31);
	vec3 lightPos = LightPos;

	vec3 normal = normalize(Normal);

	float ambientStrength = 0.1;	

	vec3 ambient = ambientStrength * vec3(1., 1., 1.);

	vec3 lightDir = normalize(lightPos - FragPos);
	
	float diff = max(dot(normal, lightDir), 0.);
	
	vec3 diffuse = diff * vec3(1., 1., 1.);

	vec3 cameraDir = normalize(CameraPos - FragPos);
	
	vec3 reflectDir = reflect(-lightDir, normal);

	float spec = pow(max(dot(cameraDir, reflectDir), 0.), 64.);	
	float specularStrength = .5;	

	vec3 specular = specularStrength * spec * vec3(1., 1., 1.);

//	vec3 result = (ambient + diffuse + specular) * vec3(texture(texSampler, vec2(texCoords.x, texCoords.y)));

	vec3 result = (ambient + diffuse + specular) * color;

	FragColor = vec4(result, 1.);
}
