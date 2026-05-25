#version 450
#define BLADE_SEGMENTS 1.f
#define BLADE_LENGTH 0.1f
layout(points) in;
layout(triangle_strip, max_vertices = 30) out;

layout(binding = 0) uniform UniformBufferObjectModel
{
mat4 model;
mat4 view;
mat4 proj;
mat4 lightSpaceMatrix;
vec3 fragColor;
vec3 cameraPos;
} ubom;

layout (location = 9) in vec3 inScale[];
layout (location = 10) in float rotation[];
layout (location = 11) in flat int vertID[];
layout (location = 12) in float DeltaTime[];
layout (location = 8) out vec3 fColor;

mat4 rotate(float angle)
{	
	return mat4(
	vec4(cos(angle),0., sin(angle), 0.),
	vec4(0.,1.,0.,0.),
	vec4(-sin(angle), 0., cos(angle), 0.),
	vec4(0., 0., 0., 1.)
);
}

mat4 scale(vec3 scale)
{
	return mat4(
	vec4(1.,0., 0., 0.),
	vec4(0.,scale.y, 0., 0.),
	vec4(0.,0., 1., 0.),
	vec4(0.,0., 0., 1.)
	);
}

void main()
{
	float angle = rotation[0];
	mat4 vp = ubom.proj * ubom.view;
	
	fColor = vec3(0.f, 0.1f, 0.f); 
	
	vec4 position = gl_in[0].gl_Position;
	float curveInfluence =(1.0f + sin(DeltaTime[0])) * fract(angle);
//	float curveInfluence = 0.0f + sin(DeltaTime[0]);

	for (float i = 0.f; i < BLADE_SEGMENTS; i += 0.1f)
	{
		float t = (BLADE_SEGMENTS - i)/20.f;
		if (i < BLADE_SEGMENTS - 0.2f)
		{
			vec4 v0 = rotate(rotation[0]) * vec4(t, (i - BLADE_LENGTH)*(i + BLADE_LENGTH), -(i - BLADE_LENGTH)*(i + BLADE_LENGTH)*curveInfluence, 0.0);
			vec4 v1 = rotate(rotation[0]) * vec4(-(t), (i - BLADE_LENGTH)*(i + BLADE_LENGTH), -(i - BLADE_LENGTH)*(i + BLADE_LENGTH)*curveInfluence, 0.0);

			gl_Position = vp * (position + ubom.model * v0 * scale(inScale[0]));	

			EmitVertex();
			
			gl_Position = vp * (position + ubom.model * v1 * scale(inScale[0]));	

			EmitVertex();
			fColor = vec3(0.f, .1f + i/2, 0.f); 
		}
		else
		{
			vec4 v2 = rotate(rotation[0]) * vec4(0.f, (i - BLADE_LENGTH)*(i + BLADE_LENGTH) * 1.5f, -(i - BLADE_LENGTH)*(i + BLADE_LENGTH)*curveInfluence, 0.0);
			gl_Position = vp * (position + ubom.model * v2 * scale(inScale[0]));	
			EmitVertex();
		}
	}

	fColor = vec3(0.f, 1.f, 0.f); 

	EndPrimitive();
}
