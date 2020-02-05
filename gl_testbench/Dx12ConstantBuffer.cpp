#include "Dx12ConstantBuffer.h"

Dx12ConstantBuffer::Dx12ConstantBuffer(std::string NAME, unsigned int location)
{
	this->name = NAME;
	this->location = location;
}

Dx12ConstantBuffer::~Dx12ConstantBuffer()
{
}

void Dx12ConstantBuffer::setData(const void * data, size_t size, Material * m, unsigned int location)
{
}

void Dx12ConstantBuffer::bind(Material *)
{
}
