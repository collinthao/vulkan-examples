#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 0) out vec3 fragColor;
layout (location = 1) in vec4 inColor;

layout(binding = 0) uniform UniformBufferObjectModel
{
mat4 model;
mat4 view;
mat4 proj;
mat4 lightSpaceMatrix;
vec3 fragColor;
vec3 cameraPos;
float deltaTime;
} ubom;

void main() 
{

    gl_Position = ubom.proj * ubom.view * ubom.model * vec4(inPosition, 1.0);
    fragColor = inColor.rgb;
}
