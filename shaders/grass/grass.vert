#version 450

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

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec3 pos;
layout(location = 5) in vec3 inScale;
layout(location = 6) in float inRot;
layout(location = 7) in int id;

layout(location = 0) out vec3 FragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 Normal;
layout(location = 3) out vec3 FragPos;
layout(location = 6) out vec3 CameraPos;
layout(location = 7) out vec4 FragPosLightSpace;
layout(location = 9) out vec3 scale;
layout(location = 10) out float rotation;
layout(location = 11) out flat int vertID;
layout(location = 12) out float DeltaTime;

void main()
{
    gl_Position = vec4(inPosition + pos, 1.0);
    fragTexCoord = inTexCoord;
    Normal = mat3(transpose(inverse(ubom.model))) * inNormal;
    FragPos = vec3(ubom.model * vec4(inPosition, 1.0));
    FragColor = pos;
    CameraPos = ubom.cameraPos;
    FragPosLightSpace =  ubom.lightSpaceMatrix * vec4(FragPos, 1.0);
    scale = inScale;
    rotation = inRot;
    vertID = id;
    DeltaTime = ubom.deltaTime;
}
