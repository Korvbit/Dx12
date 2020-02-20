#pragma once
#include <unordered_map>
#include "Mesh.h"
#include "Dx12/functions.h"
#include "Dx12VertexBuffer.h"
#include "Dx12IndexBuffer.h"
#include <d3d12.h>
#include <dxgi1_5.h>
#include <D3Dcompiler.h>


class Dx12Mesh :
	public Mesh
{
public:
	Dx12Mesh();
	Dx12Mesh(ID3D12Device* rendererDevice);
	~Dx12Mesh();

	void createCube();
	void createTriangle();
	void createQuad();

private:
	ID3D12Device* device;
	VertexBuffer* pos;
	VertexBuffer* nor;
	VertexBuffer* uvs;
	VertexBuffer* ind;
};