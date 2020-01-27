struct VSin {
	float3 pos		: POSITION;
	float3 color	: COLOR;
};

struct VSout {
	float4 pos		: SV_POSITION;
	float3 color	: COLOR;
};

cbuffer CB : register(b0) {
	float R, G, B, A;
}

VSout VS_main(VSin input, uint index : SV_VertexID) {
	VSout output = (VSout)0;
	output.pos = float4(input.pos, 1.0f);
	output.color = float4(input.color, 1.0f);

	return output;
}