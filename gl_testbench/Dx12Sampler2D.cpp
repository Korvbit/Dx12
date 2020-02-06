#include "Dx12Sampler2D.h"

Dx12Sampler2D::Dx12Sampler2D(ID3D12Device* rendererDevice)
{
	device = rendererDevice;
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 0;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;


	D3D12_DESCRIPTOR_HEAP_DESC heapDescriptorDesc = {};
	heapDescriptorDesc.NumDescriptors = 1;
	heapDescriptorDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDescriptorDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	device->CreateDescriptorHeap(&heapDescriptorDesc, IID_PPV_ARGS(&descriptorHeap));
}

Dx12Sampler2D::~Dx12Sampler2D()
{
}

void Dx12Sampler2D::setMagFilter(FILTER filter)
{
}

void Dx12Sampler2D::setMinFilter(FILTER filter)
{
}

void Dx12Sampler2D::setWrap(WRAPPING s, WRAPPING t)
{
	samplerDesc.AddressU = s ? D3D12_TEXTURE_ADDRESS_MODE_CLAMP : D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = t ? D3D12_TEXTURE_ADDRESS_MODE_CLAMP : D3D12_TEXTURE_ADDRESS_MODE_WRAP;

	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

	device->CreateSampler(&samplerDesc, descriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

ID3D12DescriptorHeap * Dx12Sampler2D::getDescriptorHeap()
{
	return descriptorHeap;
}
