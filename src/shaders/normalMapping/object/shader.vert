#version 450 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec2 inTexCoord;

layout(binding = 0) uniform ObjectUniform
{
	mat4 model;
	mat4 view;
	mat4 proj;
	vec3 cameraPos;
	vec3 lightPos;
} ud;

layout(location = 0) out vec2 texCoords;
layout(location = 1) out vec3 FragPos;
layout(location = 2) out vec3 Normal;
layout(location = 3) out vec3 CameraPos;
layout(location = 4) out vec3 LightPos;

void main()
{
	gl_Position = ud.proj * ud.view * ud.model * vec4(inPosition, 1.);
	texCoords = inTexCoord;
	FragPos = vec3(ud.model * vec4(inPosition, 1.));
	Normal = mat3(transpose(inverse(ud.model))) * inNormal;
	CameraPos = ud.cameraPos;
	LightPos = ud.lightPos;
}
