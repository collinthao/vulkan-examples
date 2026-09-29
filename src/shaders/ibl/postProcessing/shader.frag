#version 450

layout (location = 0) out vec4 fragColor;
layout (location = 0) in vec2 texCoords ;

layout (binding = 0) uniform sampler2D position;
layout (binding = 1) uniform sampler2D normal;
layout (binding = 2) uniform sampler2D albedo;
layout (binding = 3) readonly buffer DeferredUniform
{
	vec4 cameraPos;
} du;

layout (binding = 4) uniform sampler2D roughnessAndMetallic;

const float PI = 3.14159265359;

vec3 lightPositions[4] = 
{
	vec3(0., 0., 1.),
	vec3(1., 5., 1.),
	vec3(10., 10., 1.),
	vec3(6., 0., 1.)
};

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0., 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}
void main()
{
	vec3 FragPos = texture(position, texCoords).rgb;	
	vec3 N = texture(normal, texCoords).rgb;	
	vec4 AlbedoStage = texture(albedo, texCoords);	
	vec3 Albedo = AlbedoStage.rgb;	
	vec3 RoughnessAndMetallic = texture(roughnessAndMetallic, texCoords).rgb;	
	float roughness = RoughnessAndMetallic.g;	
	float metallic = RoughnessAndMetallic.r;	
	float spec = AlbedoStage.a;
	
//	float roughness = 0.2;	
//	float metallic = 1.;	

	vec3 lightColor = vec3(150.);
	vec3 Lo = vec3(0.);	
	vec3 V = normalize(du.cameraPos.xyz - FragPos);
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, Albedo, metallic);

	for (int i = 0; i < 4; i++)
	{
		vec3 lightPos = lightPositions[i];

		vec3 L = normalize(lightPos - FragPos);
		vec3 H = normalize(V + L);

		float distance = length(lightPos - FragPos);
		float attenuation = 1.0/(distance*distance);
		vec3 radiance = lightColor * attenuation;

		vec3 F = fresnelSchlick(max(dot(H, V), 0.), F0);
		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness); 

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		
		kD *= 1.0 - metallic;

		vec3 numerator    = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0)  + 0.0001;
		vec3 specular = numerator / denominator;  
		
		
		float NdotL = max(dot(N, L), 0.);
		Lo += (kD * Albedo / PI + specular) * radiance * NdotL;	
	};
	
	vec3 ambient = vec3(0.03) * Albedo * 1.0;
	vec3 color = ambient + Lo;

	color = color/(color + vec3(1.));
	color = pow(color, vec3(1./2.2));

	vec3 result = (color * spec) + ((1. - spec) * FragPos);

	fragColor = vec4(result, 1.);
}
