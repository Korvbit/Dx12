#pragma once
#define INDEXBUFF 0
#define TEXCOORD 1
#define NORMAL 2
#define POSITION 3

#define TRANSLATION 5

#define DIFFUSE_TINT 6

#define DIFFUSE_SLOT 0

typedef union {
	struct { float x, y, z, w; };
	struct { float r, g, b, a; };
} float4;

typedef union {
	struct { float x, y, z; };
	struct { float r, g, b; };
} float3;

typedef union {
	struct { float x, y; };
	struct { float u, v; };
} float2;

enum RS_INDICES
{
	RS_TEXTURES,
	RS_SAMPLERS,
	RS_CB_WVP,
	RS_CB_COLOR,
	RS_SRV_KEYFRAME_CURRENT,
	RS_SRV_KEYFRAME_NEXT,
	RS_UAV_MESH_RESULT,
	RS_CONSTANT_T,
	RS_PARAM_COUNT
};