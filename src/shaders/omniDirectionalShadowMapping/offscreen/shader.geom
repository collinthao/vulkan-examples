#version 450 core

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

layout(binding = 0) uniform OffscreenUniform
{
	mat4 model;
	mat4 projection;
	mat4 transforms[6];
	vec3 lightPos;
} ud;

layout (location = 0) out vec4 FragPos;
layout (location = 1) out vec3 LightPos;

void main()
{
	for (int face = 0; face < 6; ++face)
	{
		gl_Layer = face;
		for (int i = 0; i < 3; ++i)
		{
			FragPos =  gl_in[i].gl_Position;
			LightPos = ud.lightPos;	
			gl_Position = ud.transforms[face] * FragPos;
			EmitVertex();
		};
		EndPrimitive();
	};

}
