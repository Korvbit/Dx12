#include "Dx12RenderState.h"
#include <string>

Dx12RenderState::Dx12RenderState(ID3D12Device* rendererDevice)
{
	device = rendererDevice;

	dsopDesc.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	dsopDesc.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	dsopDesc.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	dsopDesc.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	dsDesc.StencilEnable = false;
	dsDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	dsDesc.FrontFace = dsopDesc;
	dsDesc.BackFace = dsopDesc;
}


Dx12RenderState::~Dx12RenderState()
{
}

void Dx12RenderState::setWireFrame(bool wireframe)
{
	fillMode = wireframe ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
}

void Dx12RenderState::set()
{
}

D3D12_FILL_MODE Dx12RenderState::getFillMode()
{
	return fillMode;
}

D3D12_DEPTH_STENCIL_DESC * Dx12RenderState::getStencilDesc()
{
	return &dsDesc;
}

D3D12_DEPTH_STENCILOP_DESC * Dx12RenderState::getStencilOpDesc()
{
	return &dsopDesc;
}

ID3D12PipelineState * Dx12RenderState::getPSO()
{
	return pipelineStateObject;
}

void Dx12RenderState::CreatePipelineState(ID3DBlob * vertexBlob, ID3DBlob * pixelBlob, ID3D12RootSignature* rootSignature)
{
	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = inputElementDesc;
	inputLayoutDesc.NumElements = ARRAYSIZE(inputElementDesc);

	// Create a pipeline state
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsd = {};

	// Specify pipeline stages
	gpsd.pRootSignature = rootSignature;
	gpsd.InputLayout = inputLayoutDesc;
	gpsd.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsd.VS.pShaderBytecode = reinterpret_cast<void*>(vertexBlob->GetBufferPointer());
	gpsd.VS.BytecodeLength = vertexBlob->GetBufferSize();
	gpsd.PS.pShaderBytecode = reinterpret_cast<void*>(pixelBlob->GetBufferPointer());
	gpsd.PS.BytecodeLength = pixelBlob->GetBufferSize();

	// Specify render target and depthstencil usage
	gpsd.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsd.NumRenderTargets = 1;

	gpsd.SampleDesc.Count = 1;
	gpsd.SampleMask = UINT_MAX;

	// Specify rasterizer behaviour
	gpsd.RasterizerState.FillMode = fillMode;
	gpsd.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;

	// Specify blend descriptions
	D3D12_RENDER_TARGET_BLEND_DESC defaultRTdesc = {
		false, false,
		D3D12_BLEND_ONE,D3D12_BLEND_ZERO,D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE,D3D12_BLEND_ZERO,D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL
	};

	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		gpsd.BlendState.RenderTarget[i] = defaultRTdesc;

	// Specify depth testing
	gpsd.DepthStencilState = dsDesc;

	device->CreateGraphicsPipelineState(&gpsd, IID_PPV_ARGS(&pipelineStateObject));
}
