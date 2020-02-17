#pragma once

template <class T> void SafeRelease(T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

typedef union {
	struct { float x, y, z, w; };
	struct { float r, g, b, a; };
} float4;

typedef union {
	struct { float x, y; };
	struct { float u, v; };
} float2;