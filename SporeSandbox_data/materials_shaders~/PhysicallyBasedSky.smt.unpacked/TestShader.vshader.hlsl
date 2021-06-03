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

cVertOut main(cVertIn In) {
	cVertOut Out;
	Out.pos = float4(In.pos, 1.0);
	Out.uv = In.uv;
	return Out;
};