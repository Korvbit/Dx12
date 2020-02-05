#include "Dx12Material.h"

Dx12Material::Dx12Material(const std::string & name, ID3D12Device* rendererDevice)
{
	device = rendererDevice;
}

Dx12Material::~Dx12Material()
{
}

void Dx12Material::setShader(const std::string & shaderFileName, ShaderType type)
{
	if (shaderFileNames.find(type) != shaderFileNames.end())
	{
		removeShader(type);
	}
	shaderFileNames[type] = shaderFileName;
}

void Dx12Material::removeShader(ShaderType type)
{
}

void Dx12Material::setDiffuse(Color c)
{
}

int Dx12Material::compileMaterial(std::string & errString)
{
	compileShader(ShaderType::VS);
	compileShader(ShaderType::PS);

	return 0;
}

void Dx12Material::addConstantBuffer(std::string name, unsigned int location)
{
	constantBuffers[location] = new Dx12ConstantBuffer(name, location, device);
}

void Dx12Material::updateConstantBuffer(const void * data, size_t size, unsigned int location)
{
	constantBuffers[location]->setData(data, size, this, location);
}

int Dx12Material::enable()
{
	return 0;
}

void Dx12Material::disable()
{
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
			"vs_5_0",
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
			"ps_5_0",
			0,
			0,
			&pixelBlob,
			nullptr
		);
	}
}
