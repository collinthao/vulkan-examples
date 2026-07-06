#version 450

layout (location = 0) out vec4 fragColor;
layout (location = 0) in vec3 position;
layout (location = 1) flat in int type;
layout (binding = 0) uniform samplerCube cubemapTexture;
layout (binding = 2) uniform samplerCube cubemapDirtTexture;
layout (binding = 3) uniform samplerCube cubemapWaterTexture;

void main()
{ 
	if (type == 0)	
	{
		fragColor = texture(cubemapDirtTexture, position);
	}
	else if (type == 1)
	{
		fragColor = texture(cubemapTexture, position);
	}
	else if(type == 2)
	{
		fragColor = texture(cubemapWaterTexture, position);
	}
}
