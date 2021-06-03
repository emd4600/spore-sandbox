struct cVertIn
{
	float3 pos : POSITION0;
	float2 uv : TEXCOORD0;
};

struct cVertOut
{
	float4 pos : POSITION0;
	float2 uv : TEXCOORD0;
};

float4 main(cVertIn In) : POSITION0 {
	return float4(In.pos, 1.0);
};