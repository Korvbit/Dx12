#pragma once
#include "Sampler2D.h"
#include <d3d12.h>
#include <dxgi1_5.h>
#include <D3Dcompiler.h>

class Dx12Sampler2D : public Sampler2D
{
public:
	Dx12Sampler2D(ID3D12Device* rendererDevice);
	~Dx12Sampler2D();
	void setWrap(WRAPPING s, WRAPPING t);

	ID3D12DescriptorHeap* getDescriptorHeap();

private:
	D3D12_SAMPLER_DESC samplerDesc;
	ID3D12Device* device;
	ID3D12DescriptorHeap* descriptorHeap;
};

