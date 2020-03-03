#include "Dx12ConstantBuffer.h"

Dx12ConstantBuffer::Dx12ConstantBuffer(unsigned int location, ID3D12Device* deviceIn)
{
	this->location = location;

	device = deviceIn;

	UINT cbSizeAligned = 1024 * 64;
	if (location == DIFFUSE_SLOT)
	{
		cbSizeAligned = (sizeof(4 * sizeof(float)) + 255) & ~255;
	}
	else if (location == TRANSLATION)
	{
		cbSizeAligned = (sizeof(float4) + 255) & ~255;
	}

	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = cbSizeAligned;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadHeap)
	);

	D3D12_RANGE readRange = { 0, 0 };
	uploadHeap->Map(0, &readRange, reinterpret_cast<void**>(&mappedBuffer));
}

Dx12ConstantBuffer::~Dx12ConstantBuffer()
{
	uploadHeap->Unmap(0, nullptr);
	uploadHeap->Release();
}

void Dx12ConstantBuffer::setData(const void * data, size_t size, Material * m, unsigned int location)
{
	memcpy(mappedBuffer, data, size);
}

ID3D12Resource * Dx12ConstantBuffer::getUploadHeap()
{
	return uploadHeap;
}

UINT8 * Dx12ConstantBuffer::getMappedBuffer()
{
	return mappedBuffer;
}
