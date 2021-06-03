struct cFragIn
{
	float4 pos : POSITION0;
	float2 uv : TEXCOORD0;
};

extern uniform sampler2D tex;

float4 main(cFragIn In) : COLOR0
{
	float4 color = tex2D(tex, In.uv);
	//return color;
	return float4(abs(sin(color.x)), abs(sin(color.y)), abs(sin(color.z)), 1.0);
}