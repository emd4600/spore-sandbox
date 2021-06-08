struct cVertIn {
	float3 position : POSITION;
	float2 texcoord : TEXCOORD; 
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
};

struct cVertOut {
	float4 position : POSITION;
	// using POSITION2 bugs PIX, so we will use texcoord instead
	float4 worldPosition : TEXCOORD1;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 texcoord : TEXCOORD;
};

extern uniform float4x4 modelToWorld;
extern uniform float4x4 modelToClip;

cVertOut main(cVertIn In) {
	cVertOut Out;
	
	float3 normal = In.normal * 1.0/127.0 - 1;
	float3 tangent = In.tangent * 1.0/127.0 - 1;

	Out.position = mul(float4(In.position, 1.0), modelToClip);
	//Out.worldPosition = mul(float4(In.position, 1.0), modelToClip);
	Out.worldPosition = mul(float4(In.position, 1.0), modelToWorld);
	Out.normal = mul(float4(normal, 0.0), modelToWorld);
	Out.tangent = mul(float4(tangent, 0.0), modelToWorld);
	Out.texcoord = In.texcoord;
	
	return Out;
}