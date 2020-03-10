//StructuredBuffer<float4> posCurrent : register(t0, space2);
//StructuredBuffer<float4> norCurrent : register(t1, space2);
//StructuredBuffer<float4> posNext : register(t2, space2);
//StructuredBuffer<float4> norNext : register(t3, space2);
//RWBuffer<float4> posOut : register(u0, space2);
//cbuffer constants : register(b0, space2) {
//	float t;
//}

[numthreads(1, 1, 1)]
void CS_main() {//uint index : SV_VertexID) {
	//posOut[index] = lerp(posCurrent[index], posNext[index], t);
}