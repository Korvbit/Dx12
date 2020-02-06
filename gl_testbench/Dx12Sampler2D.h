#pragma once
#include "Sampler2D.h"
#include <d3d12.h>
#include <dxgi1_5.h>
#include <D3Dcompiler.h>

class Dx12Sampler2D : public Sampler2D
{
public:
	Dx12Sampler2D();
	~Dx12Sampler2D();
	void setMagFilter(FILTER filter);
	void setMinFilter(FILTER filter);
	void setWrap(WRAPPING s, WRAPPING t);

private:
	D3D12_SAMPLER_DESC samplerDesc;
};

