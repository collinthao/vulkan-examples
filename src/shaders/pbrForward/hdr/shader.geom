#version 450 core

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

layout (location = 0) in vec3 inPosition[];
layout (location = 0) out vec3 outPosition;

layout(binding=0) uniform  HDRUniformData
{
	mat4 view[6];
	mat4 model;
	mat4 proj;
} ud;

void main()
{
	for (int face = 0; face < 6; ++face)
	{
		gl_Layer = face;
		for (int i = 0; i < 3; ++i)
		{
			gl_Position = ud.view[face] * gl_in[i].gl_Position;
			outPosition = inPosition[i];
			EmitVertex();
		};
		EndPrimitive();
	};
}
