#version 450 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;

layout(binding = 0) uniform ObjectUniform
{
	mat4 model;
	mat4 view;
	mat4 proj;
	mat4 lightSpace;
	vec3 cameraPos;
	vec3 lightDir;
	vec3 lightPos;
} u;

layout(location = 0) out vec2 texCoords;
layout(location = 1) out vec4 FragPos;
layout(location = 2) out vec3 Normal;
layout(location = 3) out vec3 CameraPos;
layout(location = 4) out vec3 LightDir;
layout(location = 5) out vec4 LightSpace;
layout(location = 6) out vec3 LightPos;

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main()
{
	gl_Position = u.proj * u.view * u.model * vec4(inPosition, 1.);
	texCoords = inTexCoord;
	FragPos = u.model * vec4(inPosition, 1.);
	Normal = mat3(transpose(inverse(u.model))) * inNormal;
	CameraPos = u.cameraPos;
	LightDir = u.lightDir;
	LightSpace = (biasMat * u.lightSpace * u.model) * vec4(inPosition, 1.0);
	LightPos = u.lightPos;
}
