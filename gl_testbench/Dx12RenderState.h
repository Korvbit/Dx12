#pragma once
#include "RenderState.h"

#include <d3d12.h>
#include <dxgi1_5.h>
#include <D3Dcompiler.h>

class Dx12RenderState :
	public RenderState
{
public:
	Dx12RenderState(ID3D12Device* rendererDevice);
	~Dx12RenderState();

	void setWireFrame(bool wireframe);

	void set();

	D3D12_FILL_MODE getFillMode();
	D3D12_DEPTH_STENCIL_DESC* getStencilDesc();
	D3D12_DEPTH_STENCILOP_DESC* getStencilOpDesc();
	ID3D12PipelineState* getPSO();

	void CreatePipelineState(ID3DBlob* vertexBlob, ID3DBlob* pixelBlob, ID3D12RootSignature* rootSignature);

private:
	D3D12_FILL_MODE fillMode;
	D3D12_DEPTH_STENCILOP_DESC dsopDesc;
	D3D12_DEPTH_STENCIL_DESC dsDesc;
	ID3D12Device* device;
	ID3D12PipelineState* pipelineStateObject;
};

