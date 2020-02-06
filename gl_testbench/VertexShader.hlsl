struct VSin {
	float3 pos			: POSITION;
	float3 nor			: NORMAL;
	float2 texCoords	: TEXCOORD;
};

struct VSout {
	float4 pos			: SV_POSITION;
	float2 texCoords	: TEXCOORD;
};

cbuffer CB : register(b0) {
	float X, Y, Z, A;
}

VSout VS_main(VSin input, uint index : SV_VertexID) {
	VSout output = (VSout)0;
	output.pos = float4(input.pos, 1.0f) + float4(X, Y, Z, A);
	output.texCoords = input.texCoords;

	return output;
}