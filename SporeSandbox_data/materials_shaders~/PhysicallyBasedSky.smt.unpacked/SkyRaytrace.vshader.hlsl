#include "PBSkyDefines.hlsl"

struct cVertIn
{
	float3 pos : POSITION0;
	float2 uv : TEXCOORD0;
};

struct cVertOut
{
	float4 pos : POSITION0;
	float3 viewRay : NORMAL;
};

extern uniform InputParameters customParams;

cVertOut main(cVertIn In)
{
	cVertOut Out;
	Out.pos = float4(In.pos, 1.0);
	Out.viewRay = mul(customParams.viewToWorld, 
	  float4(mul(customParams.clipToView, float4(In.pos.x, In.pos.y, 0.0, 1.0)).xyz, 0.0)).xyz;
	return Out;
}