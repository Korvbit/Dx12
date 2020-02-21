#pragma once
#define POSITION 0
#define NORMAL 1
#define TEXCOORD 2
#define INDEXBUFF 4

#define TRANSLATION 5
#define TRANSLATION_NAME "TranslationBlock"

#define DIFFUSE_TINT 6
#define DIFFUSE_TINT_NAME "DiffuseColor"

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