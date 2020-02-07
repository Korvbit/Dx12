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
	return float4(R, G, B, A);
	return tex.Sample(samp, input.texCoords);
}