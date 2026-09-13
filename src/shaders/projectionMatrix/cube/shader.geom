#version 450

layout(points) in;
layout(line_strip, max_vertices=2) out;

layout(location = 0) in mat4 Model[];
layout(location = 4) in mat4 View[];
layout(location = 8) in mat4 Proj[];

void main()
{
	gl_Position = Proj[0] * View[0] * (((gl_in[0].gl_Position + vec4(1.)))*Model[0]);
	EmitVertex();                                          
	gl_Position = Proj[0] * View[0] * (((gl_in[0].gl_Position) * Proj[0])*Model[0]);
	EmitVertex();
	EndPrimitive();
}
