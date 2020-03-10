struct VSin {
	float4 pos			: POSITION;
	float4 nor			: NORMAL;
	float2 texCoords	: TEXCOORD;
};

struct VSout {
	float4 pos			: SV_POSITION;
	float2 texCoords	: TEXCOORD;
};

cbuffer wvpMatrix : register(b0, space0) {
	float4x4 wvpMat;
}

StructuredBuffer<float4> posCurrent : register(t0, space2);
StructuredBuffer<float4> norCurrent : register(t1, space2);
StructuredBuffer<float4> posNext : register(t2, space2);
StructuredBuffer<float4> norNext : register(t3, space2);
RWBuffer<float4> posOut : register(u0, space2);
cbuffer constants : register(b0, space2) {
	float t;
}

VSout VS_main(VSin input, uint index : SV_VertexID) {
	VSout output = (VSout)0;
	//output.pos = mul(input.pos, wvpMat);
	output.texCoords = input.texCoords;

	output.pos = mul(lerp(posCurrent[index], posNext[index], t), wvpMat);

	//float4 pos = posData[index];

	return output;
}