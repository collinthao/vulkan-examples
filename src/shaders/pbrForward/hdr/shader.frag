#version 450 core

layout(location = 0) out vec4 FragColor;

layout(binding = 1) uniform sampler2D cubemap;
layout(location = 0) in vec3 inPosition;

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
	FragColor = vec4(vec3(cubemapTexture), 1.);
}
