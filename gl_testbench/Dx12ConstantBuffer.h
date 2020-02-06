#pragma once

#include <d3d12.h>
#include <dxgi1_5.h>
#include <D3Dcompiler.h>
#include <vector>
#include "IA.h"
#include "ConstantBuffer.h"

typedef union {
	struct { float x, y, z, w; };
	struct { float r, g, b, a; };
} float4;

class Dx12ConstantBuffer : public ConstantBuffer
{
public:
	Dx12ConstantBuffer(std::string NAME, unsigned int location, ID3D12Device* deviceIn);
	~Dx12ConstantBuffer();
	// set data will update the buffer associated, including whatever is necessary to
	// update the GPU memory.
	void setData(const void* data, size_t size, Material* m, unsigned int location);
	void bind(Material*);
	
	std::string name;
	unsigned int location;

private:
	ID3D12Device* device;
	ID3D12Resource* uploadHeap;
	UINT8* mappedBuffer;
};