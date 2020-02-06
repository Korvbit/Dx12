#include "Dx12Sampler2D.h"

Dx12Sampler2D::Dx12Sampler2D()
{
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 0;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
}

Dx12Sampler2D::~Dx12Sampler2D()
{
}

void Dx12Sampler2D::setMagFilter(FILTER filter)
{
}

void Dx12Sampler2D::setMinFilter(FILTER filter)
{
}

void Dx12Sampler2D::setWrap(WRAPPING s, WRAPPING t)
{
	samplerDesc.AddressU = s ? D3D12_TEXTURE_ADDRESS_MODE_CLAMP : D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = t ? D3D12_TEXTURE_ADDRESS_MODE_CLAMP : D3D12_TEXTURE_ADDRESS_MODE_WRAP;

	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
}
