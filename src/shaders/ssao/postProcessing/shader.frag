#version 450

layout (location = 0) out vec4 fragColor;
layout (location = 0) in vec2 texCoords ;

layout (binding = 0) uniform sampler2D position;
layout (binding = 1) uniform sampler2D normal;
layout (binding = 2) uniform sampler2D albedo;
layout (binding = 3) uniform sampler2D randomNoise;
layout (binding = 4) readonly buffer DeferredUniform
{
	vec4 kernelSamples[64];	
	mat4 proj;
	vec4 extent;
} du;

void main()
{
	vec2 TexCoords = texCoords;
	vec3 FragPos = texture(position, TexCoords).rgb;
	vec3 Normal = texture(normal, TexCoords).rgb;	
	vec3 randomVec = texture(randomNoise, TexCoords * vec2(du.extent.x/4.0, du.extent.y/4.0)).rgb;	

	vec3 tangent   = normalize(randomVec - Normal * dot(randomVec, Normal));
	vec3 bitangent = cross(Normal, tangent);
	mat3 TBN       = mat3(tangent, bitangent, Normal);  

	float occlusion = 0.;
	for (int i = 0; i < 64; i++)
	{
		vec3 samplePos = TBN * vec3(du.kernelSamples[i]); 	
		samplePos = FragPos + samplePos * 0.5;
		
		vec4 offset = vec4(samplePos, 1.0);
		offset = du.proj * offset;
		offset.xyz /= offset.w;
		offset.xyz /= offset.xyz * 0.5 + 0.5;
		
		float sampleDepth = texture(position, offset.xy).z;	
		
		float rangeCheck = smoothstep(0.0, 1.0, 0.5/abs(FragPos.z - sampleDepth));

		occlusion += (sampleDepth >= samplePos.z + 0.025 ? 1.0 : 0.0) * rangeCheck;
	}	
	
	occlusion = 1.0 - (occlusion / 64);

	fragColor = vec4(vec3(occlusion), 1.);
}
