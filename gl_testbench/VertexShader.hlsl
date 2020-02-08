struct VSin {
	float4 pos			: POSITION;
	float4 nor			: NORMAL;
	float2 texCoords	: TEXCOORD;
};

struct VSout {
	float4 pos			: SV_POSITION;
	float2 texCoords	: TEXCOORD;
};

cbuffer translate : register(b0) {
	float X, Y, Z, W;
}

VSout VS_main(VSin input, uint index : SV_VertexID) {
	VSout output = (VSout)0;
	output.pos = input.pos + float4(X, Y, 1+Z, W);
	output.texCoords = input.texCoords;

	return output;
}