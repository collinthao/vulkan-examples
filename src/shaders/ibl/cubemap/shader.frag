#version 450 core

layout(location = 0) out vec4 PositionColor;
layout(location = 1) out vec4 NormalColor;
layout(location = 2) out vec4 AlbedoColor;
layout(location = 3) out vec4 RoughnessAndMetallic;

layout(location = 0) in vec3 inPosition;

layout(binding = 1) uniform sampler2D cubemap;

const vec2 invAtan = vec2(0.1591,0.3183);
vec2 SampleSphericalMap(vec3 v)
{
	vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
	uv *= invAtan;
	uv += 0.5;
	return uv;
}

void main()
{
	vec2 uv = SampleSphericalMap(normalize(inPosition));
	vec4 cubemapTexture = texture(cubemap, vec2(uv.x, -uv.y));
	PositionColor = vec4(vec3(cubemapTexture), 1.);
	NormalColor = vec4(1.);
	AlbedoColor = vec4(vec3(cubemapTexture),0.);
	RoughnessAndMetallic = vec4(1.);
}
