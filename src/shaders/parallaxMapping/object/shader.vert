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

layout(location = 0) out vec2 TexCoords;
layout(location = 1) out vec3 FragPos;
layout(location = 2) out vec3 CameraPos;
layout(location = 3) out vec3 LightPos;

void main()
{
	TexCoords = inTexCoord;
	//TexCoords.y = 1.0 - TexCoords.y;
	vec3 T = normalize(vec3(ud.model * vec4(inTangent, 0.0)));
	vec3 N = normalize(vec3(ud.model * vec4(inNormal, 0.0)));
	vec3 B = normalize(vec3(ud.model * vec4(inBitangent, 0.0)));
	mat3 TBN = transpose(mat3(T, B, N));

	FragPos = TBN * vec3(ud.model * vec4(inPosition, 1.));
	//FragPos.x *= -1.;
	//FragPos.y *= -1.;
	CameraPos = TBN * ud.cameraPos;
	LightPos = TBN * ud.lightPos;

	gl_Position = ud.proj * ud.view * ud.model * vec4(inPosition, 1.);
}
