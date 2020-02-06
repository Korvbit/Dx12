#include "Dx12VertexBuffer.h"

Dx12VertexBuffer::Dx12VertexBuffer(size_t size, VertexBuffer::DATA_USAGE usage, ID3D12Device* rendererDevice)
{
	device = rendererDevice;

	D3D12_HEAP_PROPERTIES hp = {};
	hp.Type = D3D12_HEAP_TYPE_UPLOAD;
	hp.CreationNodeMask = 1;
	hp.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC rd = {};
	rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	rd.Width = size;
	rd.Height = 1;
	rd.DepthOrArraySize = 1;
	rd.MipLevels = 1;
	rd.SampleDesc.Count = 1;
	rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// Creates a heap of adequate size and a corresponding resource mapped to the heap
	device->CreateCommittedResource(
		&hp,
		D3D12_HEAP_FLAG_NONE,
		&rd,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&buffer)
	);


	/*void* dataBegin = nullptr;
	D3D12_RANGE range = { 0,0 };
	buffer->Map(0, &range, &dataBegin);
	buffer->Unmap(0, nullptr);*/

	view.BufferLocation = buffer->GetGPUVirtualAddress();
	view.StrideInBytes = size / 100;
	view.SizeInBytes = size;
}

Dx12VertexBuffer::~Dx12VertexBuffer()
{
}

void Dx12VertexBuffer::setData(const void * data, size_t size, size_t offset)
{
	UINT8* gpuAddress;
	D3D12_RANGE range = { 0,0 };
	buffer->Map(0, &range, reinterpret_cast<void**>(&gpuAddress));
	memcpy(gpuAddress + offset, data, size);
	buffer->Unmap(0, nullptr);
}

void Dx12VertexBuffer::bind(size_t offset, size_t size, unsigned int location)
{
}

void Dx12VertexBuffer::unbind()
{
}

size_t Dx12VertexBuffer::getSize()
{
	return size_t();
}
