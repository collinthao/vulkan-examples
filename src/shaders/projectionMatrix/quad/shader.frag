#version 450

layout (location = 0) out vec4 fragColor;
layout (location = 0) in vec2 texCoords ;

layout (binding = 0) uniform sampler2D texture_sampler;

void main()
{
    vec4 tex = texture(texture_sampler, texCoords);
    fragColor = vec4(tex.rgb, 1.); 
}
