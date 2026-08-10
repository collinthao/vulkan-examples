#version 450 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;

layout(binding = 0) uniform ShadowMapUniform
{
	mat4 model;
	mat4 view;
	mat4 proj;
} ud;

layout(location = 0) out vec2 texCoords;
layout(location = 1) out vec3 FragPos;
layout(location = 2) out vec3 Normal;

void main()
{
	gl_Position = ud.proj * ud.view * ud.model * vec4(inPosition, 1.);
	texCoords = inTexCoord;
	FragPos = vec3(ud.model * vec4(inPosition, 1.));
	Normal = mat3(transpose(inverse(ud.model))) * inNormal;
}
