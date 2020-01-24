struct PSin {
	float4 pos		: SV_POSITION;
	float3 color	: COLOR;
};

float4 main(PSin input) : SV_TARGET0 {
	return float4(input.color, 1.0f);
}