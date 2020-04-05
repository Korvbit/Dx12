struct VSin {
	float4 pos			: POSITION;
	float4 nor			: NORMAL;
	float2 texCoords	: TEXCOORD;
};

struct VSout {
	float4 pos			: SV_POSITION;
	float4 nor			: NORMAL;
	float2 texCoords	: TEXCOORD;
};

cbuffer wvpMatrix : register(b0, space0) {
	float4x4 wvpMat;
}

RWStructuredBuffer<float4> posIn : register(u0, space2);
RWStructuredBuffer<float4> norIn : register(u1, space2);

VSout VS_main(VSin input, uint index : SV_VertexID) {
	VSout output = (VSout)0;
	output.texCoords = input.texCoords;

	output.pos = mul(posIn[index], wvpMat);
	output.nor = mul(norIn[index], wvpMat);

	return output;
}