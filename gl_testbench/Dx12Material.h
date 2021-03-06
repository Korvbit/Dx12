#pragma once

#include <d3d12.h>
#include <dxgi1_5.h>
#include <D3Dcompiler.h>
#include <vector>
#include "Material.h"
#include "Dx12ConstantBuffer.h"

class Dx12Renderer;

class Dx12Material :
	public Material
{
	friend Dx12Renderer;

public:
	Dx12Material(ID3D12Device* rendererDevice);
	~Dx12Material();

	/*
	 * Compile and link all shaders
	 * Returns 0  if compilation/linking succeeded.
	 * Returns -1 if compilation/linking fails.
	 * Error is returned in errString
	 * A Vertex and a Fragment shader MUST be defined.
	 * If compileMaterial is called again, it should RE-COMPILE the shader
	 * In principle, it should only be necessary to re-compile if the defines set
	 * has changed.
	*/
	int compileMaterial(std::string& errString);
	void addConstantBuffer(unsigned int location);
	void updateConstantBuffer(const void* data, size_t size, unsigned int location);

	ID3DBlob* vertexBlob;
	ID3DBlob* pixelBlob;

	std::map<unsigned int, Dx12ConstantBuffer*> constantBuffers;

private:
	void compileShader(ShaderType type);

	ID3D12Device* device;
};