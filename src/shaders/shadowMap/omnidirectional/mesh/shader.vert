#version 450 core

layout(location = 0) in vec3 inPosition;

layout(binding = 0) uniform UniformBufferObjectModel
{
mat4 model;
mat4 view;
mat4 proj;
vec3 fragColor;
vec3 cameraPos;
} ubom;

void main()
{
    gl_Position = ubom.model * vec4(inPosition, 1.0);
}
