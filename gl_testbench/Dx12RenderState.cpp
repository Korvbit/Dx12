#include "Dx12RenderState.h"



Dx12RenderState::Dx12RenderState()
{
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
