#pragma once
#define NORMAL 0
#define TEXCOORD 1
#define INDEXBUFF 2
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