Texture2D tex : register(t0);
SamplerState samp : register(s0);

struct PSin {
	float4 pos			: SV_POSITION;
	float2 texCoords	: TEXCOORD;
};

float4 PS_main(PSin input) : SV_TARGET0 {
	return tex.Sample(samp, input.texCoords);
}