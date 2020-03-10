#pragma once
#include "VertexBuffer.h"
#include <d3d12.h>
#include <dxgi1_5.h>
#include <D3Dcompiler.h>

class Dx12VertexBuffer : public VertexBuffer
{
public:
	Dx12VertexBuffer(size_t size, int numEntries, ID3D12Device* rendererDevice);
	~Dx12VertexBuffer();
	void setData(const void* data, size_t size, size_t offset);
	D3D12_VERTEX_BUFFER_VIEW* getView();
	ID3D12Resource* getUploadHeap();

private:
	ID3D12Device* device;
	ID3D12Resource* buffer;
	D3D12_VERTEX_BUFFER_VIEW view;
};