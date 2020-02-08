Texture2D tex : register(t0);
SamplerState samp : register(s0);

cbuffer diffuse : register(b1) {
	float R, G, B, A;
}

struct PSin {
	float4 pos			: SV_POSITION;
	float2 texCoords	: TEXCOORD;
};

float4 PS_main(PSin input) : SV_TARGET0 {
	if (R != 1.0f || G != 1.0f || B != 1.0f)
	{
		return float4(R, G, B, A);
	}
	else
	{
		return tex.Sample(samp, input.texCoords);
	}
}