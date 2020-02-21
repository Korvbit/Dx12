#pragma once
#include <unordered_map>
#include "Mesh.h"
#include "Dx12/functions.h"
#include "Dx12VertexBuffer.h"
#include "Dx12IndexBuffer.h"
#include <d3d12.h>
#include <dxgi1_5.h>
#include <D3Dcompiler.h>
#include "Camera.h"
#include <DirectXMath.h>


class Dx12Mesh :
	public Mesh
{
public:
	Dx12Mesh(
				ID3D12Device* rendererDevice, 
				float4 translate =	{ 0.0f, 0.0f, 0.0f, 0.0f },
				float4 rotate =		{ 0.0f, 0.0f, 0.0f, 0.0f },
				float4 scale =		{ 1.0f, 1.0f, 1.0f, 0.0f }
	);
	~Dx12Mesh();

	void Update(float4 translate = { 0.0f, 0.0f, 0.0f, 0.0f },
				float4 rotate = { 0.0f, 0.0f, 0.0f, 0.0f },
				float4 scale = { 1.0f, 1.0f, 1.0f, 0.0f }
	);

	void scale(float4 scale, bool linear);
	void rotate(float4 rotate);
	void translate(float4 translate);

	void createCube();
	void createTriangle();
	void createQuad();

private:
	ID3D12Device* device;
	VertexBuffer* pos;
	VertexBuffer* nor;
	VertexBuffer* uvs;
	VertexBuffer* ind;

	DirectX::XMFLOAT4X4 scaleMatrix;
	DirectX::XMFLOAT4X4 rotationMatrix;
	DirectX::XMFLOAT4X4 translationMatrix;
};