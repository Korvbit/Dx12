#pragma once
#include "Renderer.h"

#include <d3d12.h>
#include <dxgi1_5.h>
#include <D3Dcompiler.h>
#include "d3dx12.h"
#include "functions.h"

class Dx12Renderer :
	public Renderer
{
public:
	Dx12Renderer();
	~Dx12Renderer();

	int initialize();

	ID3D12Device* device;
};

