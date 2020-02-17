#pragma once

#include "../Texture2D.h"
#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_5.h>
#include <D3Dcompiler.h>
#include "d3dx12.h"
#include "functions.h"
#include <wincodec.h>


class Dx12Texture2D :
	public Texture2D
{
public:
	Dx12Texture2D(ID3D12Device* rendererDevice, ID3D12GraphicsCommandList* rendererCommandList, ID3D12CommandQueue* rendererCommandQueue, ID3D12CommandAllocator* rendererCommandAllocator);
	~Dx12Texture2D();

	int loadFromFile(std::string filename);
	ID3D12DescriptorHeap* getDescriptorHeap();

	BYTE* imageData;
	D3D12_RESOURCE_DESC resourceDesc;
	UINT bytesPerRow;

private:
	ID3D12Device* device;
	ID3D12GraphicsCommandList* commandList;
	ID3D12CommandQueue* commandQueue;
	ID3D12CommandAllocator* commandAllocator;
	ID3D12Resource* textureBuffer;
	ID3D12Resource* textureBufferUploadHeap;
	ID3D12DescriptorHeap* descriptorHeap;
	ID3D12Fence* fence;
	UINT64 fenceValue;
	HANDLE fenceEvent;
};

DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);
int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);