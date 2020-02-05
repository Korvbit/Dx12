#pragma once
#include "RenderState.h"

#include <d3d12.h>
#include <dxgi1_5.h>
#include <D3Dcompiler.h>

class Dx12RenderState :
	public RenderState
{
public:
	Dx12RenderState();
	~Dx12RenderState();

	void setWireFrame(bool wireframe);

	void set();

	D3D12_FILL_MODE getFillMode();

private:
	D3D12_FILL_MODE fillMode;
};

