StructuredBuffer<float4> posCurrent : register(t0, space2);
StructuredBuffer<float4> norCurrent : register(t1, space2);
StructuredBuffer<float4> posNext : register(t2, space2);
StructuredBuffer<float4> norNext : register(t3, space2);
RWStructuredBuffer<float4> posOut : register(u0, space2);
RWStructuredBuffer<float4> norOut : register(u1, space2);
cbuffer constants : register(b0, space2) {
	float t;
}

[numthreads(32, 1, 1)]
void CS_main(uint3 dtID : SV_DispatchThreadID) {
	/*float4 tempVec[367];
	tempVec[dtID.x] = lerp(posCurrent[dtID.x], posNext[dtID.x], t);
*/
	posOut[dtID.x] = lerp(posCurrent[dtID.x], posNext[dtID.x], t);
	norOut[dtID.x] = lerp(norCurrent[dtID.x], norNext[dtID.x], t);
}