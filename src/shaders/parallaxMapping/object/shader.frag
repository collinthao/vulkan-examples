#version 450 core

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec2 TexCoords;
layout(location = 1) in vec3 FragPos;
layout(location = 2) in vec3 CameraPos;
layout(location = 3) in vec3 LightPos;

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler2D normSampler;
layout(binding = 3) uniform sampler2D dispSampler;

vec2 ParallaxMapping(vec2 texCoords, vec3 cameraDir);

void main()
{
	vec3 fragPos = FragPos;

	vec3 lightPos = LightPos;

	vec3 cameraDir = normalize(CameraPos - fragPos);

	vec2 texCoords = ParallaxMapping(TexCoords, cameraDir);	

	if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
	discard;

	vec3 normal = vec3(texture(normSampler, vec2(texCoords)));
	
	normal = normalize(normal * 2.0 - 1.0);
//	normal = 1.0 - normal;
	
	vec3 textureSampler = vec3(texture(texSampler, vec2(texCoords.x, texCoords.y)));

	float ambientStrength = 0.1;	

	vec3 ambient = ambientStrength * textureSampler;

	vec3 lightDir = normalize(lightPos - fragPos);
	
	float diff = max(dot(normal, lightDir), 0.);
	
	vec3 diffuse = diff * textureSampler;
	
	vec3 reflectDir = reflect(-lightDir, normal);

	float spec = pow(max(dot(cameraDir, reflectDir), 0.), 32.);

	float specularStrength = .2;	

	vec3 specular = specularStrength * spec * vec3(1.);

	vec3 result = (ambient + diffuse + specular);

	FragColor = vec4(result, 1.);
}

vec2 ParallaxMapping(vec2 texCoords, vec3 cameraDir)
{
	float height = texture(dispSampler, texCoords).r;
	
	vec2 p = cameraDir.xy * (height * .2);
	
	return texCoords - p;
}
