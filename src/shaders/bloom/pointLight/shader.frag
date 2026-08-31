#version 450

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BloomColor;
layout(location = 0) in vec3 LightColor;

void main()
{
	FragColor = vec4(LightColor, 1.);

	BloomColor = vec4(FragColor.rgb, 1.);
}
