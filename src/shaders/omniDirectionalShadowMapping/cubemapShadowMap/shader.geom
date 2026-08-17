#version 450 core

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

void main()
{
	gl_Position = vec4(1.);	
	EmitVertex();
	EndPrimitive();
}
