Texture2D tex : register(t0);
SamplerState samp : register(s0);

struct PSin {
	float4 pos			: SV_POSITION;
	float3 texCoords	: TEXCOORD;
};

float4 PS_main(PSin input) : SV_TARGET0 {
	//return tex.Sample(samp, input.texCoords);
	return float4(1.0f, 0.0f, 1.0f, 1.0f);
}