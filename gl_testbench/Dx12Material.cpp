#include "Dx12Material.h"

Dx12Material::Dx12Material(ID3D12Device* rendererDevice)
{
	device = rendererDevice;
}

Dx12Material::~Dx12Material()
{
	vertexBlob->Release();
	pixelBlob->Release();
	
	for (auto work : constantBuffers)
	{
		delete work.second;
	}
}

int Dx12Material::compileMaterial(std::string & errString)
{
	compileShader(ShaderType::VS);
	compileShader(ShaderType::PS);

	return 0;
}

void Dx12Material::addConstantBuffer(unsigned int location)
{
	constantBuffers[location] = new Dx12ConstantBuffer(location, device);
}

void Dx12Material::updateConstantBuffer(const void * data, size_t size, unsigned int location)
{
	constantBuffers[location]->setData(data, size);
}

void Dx12Material::compileShader(ShaderType type)
{
	if (type == ShaderType::VS)
	{
		// Compile VertexShader
		D3DCompileFromFile(
			L"VertexShader.hlsl",
			nullptr,
			nullptr,
			"VS_main",
			"vs_5_1",
			0,
			0,
			&vertexBlob,
			nullptr
		);
	}
	else if (type == ShaderType::PS)
	{
		// Compile PixelShader
		D3DCompileFromFile(
			L"PixelShader.hlsl",
			nullptr,
			nullptr,
			"PS_main",
			"ps_5_1",
			0,
			0,
			&pixelBlob,
			nullptr
		);
	}
}
