struct cVertOut {
	float4 position : POSITION;
	float4 worldPosition : TEXCOORD1;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 texcoord : TEXCOORD;
};

extern uniform float time;

struct PointLight {
	float4 position;
	float3 color;
};
// extern uniform PointLight hdPointLights[4];
extern uniform float4 worldCameraPosition;
extern uniform float4 customParams[2];
extern uniform struct {
	float4 dir;
	float4 color;
} dirLightsWorld[1];

sampler2D albedoMap : register(s0);
sampler2D normalMap : register(s1);
sampler2D metallicMap : register(s2);
sampler2D aoMap : register(s3);

static const float PI = 3.14159265359;

float3 ApplyNormalMap(cVertOut In)
{
	float3 tangentSpace = tex2D(normalMap, In.texcoord).rgb * 2.0 - 1.0;
	float3 T = normalize(In.tangent);
	float3 N = normalize(In.normal);
	float3 B  = normalize(cross(N, T));
	float3x3 TBN = float3x3(T, B, N);
	return mul(tangentSpace, TBN);
}

// F0 is surface reflecion at zero incidence
float3 fresnelSchlick(float cosTheta, float3 F0) 
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(float3 N, float3 H, float roughness) 
{
	float a = roughness*roughness;
	float a2 = a*a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH*NdotH;
	
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;
	
	return a2 / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) 
{
	float r = roughness + 1.0;
	float k = (r*r) / 8.0;
	float denom = NdotV * (1.0 - k) + k;
	
	return NdotV / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	
	return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

float4 main(cVertOut In) : COLOR 
{
	/*float3 albedo = customParams[0].xyz;
	float metallic = customParams[0].w;
	float roughness = customParams[1].x;
	float ao = customParams[1].y;
	float3 N = normalize(In.normal);*/
	
	float4 albedoRoughness = tex2D(albedoMap, In.texcoord);
	float3 albedo = pow(albedoRoughness.rgb, 2.2);  // Gamma correct
	float roughness = albedoRoughness.a;
	float metallic = tex2D(metallicMap, In.texcoord).a;
	float ao = tex2D(aoMap, In.texcoord).a;
	float3 N = ApplyNormalMap(In);
	
	float3 V = normalize(worldCameraPosition.xyz - In.worldPosition.xyz);
	
	// This actually depends on surface material, but this looks good enough
	// for non-metallic surfaces
	float3 F0 = 0.04;
	F0 = lerp(F0, albedo, metallic);
	
	float3 Lo = 0.0;
	for (int i = 0; i < 1; ++i) 
	{
		float3 L = normalize(dirLightsWorld[i].dir.xyz);
		float3 H = normalize(V + L);
		
		//float distance = length(hdPointLights[i].position - In.worldPosition);
		//float attenuation = 1.0 / (distance * distance);
		float attenuation = 1.0;
		float3 radiance = dirLightsWorld[i].color.xyz * attenuation;
		
		float3 F = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		
		float NdotL = max(dot(N, L), 0.0);
		
		float3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL;
		float3 specular = numerator / max(denominator, 0.001);
		
		float3 kS = F;
		float3 kD = 1.0 - kS;
		kD *= 1.0 - metallic;
		
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}
	
	float3 ambient = 0.03 * albedo * ao;
	float3 color = ambient + Lo;
	
	// HDR tonemapping
    color = color / (color + 1.0);
    // gamma correct
    color = pow(color, 1.0/2.2); 

	//color = (N + 1.0) / 2.0;
	//color = In.normal;
	return float4(color, 1.0);
	//return float4(1.0, 0.0, 0.0, 1.0);
	//return float4(normalize(dirLightsWorld[0].dir.xyz), 1.0);
	//return float4(sin(time), In.texcoord.x, In.texcoord.y, 1.0);
}