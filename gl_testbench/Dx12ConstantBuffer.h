#pragma once
#include "ConstantBuffer.h"

class Dx12ConstantBuffer : public ConstantBuffer
{
public:
	Dx12ConstantBuffer(std::string NAME, unsigned int location);
	~Dx12ConstantBuffer();
	// set data will update the buffer associated, including whatever is necessary to
	// update the GPU memory.
	void setData(const void* data, size_t size, Material* m, unsigned int location);
	void bind(Material*);
	
	std::string name;
	unsigned int location;
};