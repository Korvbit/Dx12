Texture2D tex : register(t0, space1);
SamplerState samp : register(s0, space1);

cbuffer diffuse : register(b0, space1) {
	float R, G, B, A;
}

struct PSin {
	float4 pos			: SV_POSITION;
	float4 nor			: NORMAL;
	float2 texCoords	: TEXCOORD;
};

float4 PS_main(PSin input) : SV_TARGET0 {

	return float4(R, G, B, A) + tex.Sample(samp, input.texCoords);
}