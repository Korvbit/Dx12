#pragma once

#include "../Texture2D.h"
#include <d3d12.h>
#include <dxgi1_5.h>
#include <D3Dcompiler.h>
#include "d3dx12.h"
#include "functions.h"
#include <wincodec.h>


class Dx12Texture2D :
	public Texture2D
{
public:
	Dx12Texture2D();
	~Dx12Texture2D();

	int loadFromFile(std::string filename);
	void bind(unsigned int slot) {};

	BYTE* imageData;
	D3D12_RESOURCE_DESC resourceDesc;
	UINT bytesPerRow;
};

DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);
int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);