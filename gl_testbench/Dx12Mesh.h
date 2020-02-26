#pragma once
#include <unordered_map>
#include "Mesh.h"
#include "Dx12/functions.h"
#include "Dx12VertexBuffer.h"
#include "Dx12IndexBuffer.h"
#include <d3d12.h>
#include <dxgi1_5.h>
#include <D3Dcompiler.h>
#include "Dx12Camera.h"
#include <DirectXMath.h>
#include "IA.h"


class Dx12Mesh :
	public Mesh
{
public:
	Dx12Mesh(
				ID3D12Device* rendererDevice, 
				float3 translate =	{ 0.0f, 0.0f, 0.0f },
				float3 rotate =		{ 0.0f, 0.0f, 0.0f },
				float3 scale =		{ 1.0f, 1.0f, 1.0f }
	);
	~Dx12Mesh();

	void Update(Camera* camera);

	void scaleMesh(float3 scale);
	void rotateMesh(float3 rotate);
	void translateMesh(float3 translate);

	void setScale(float3 scale);
	void setRotation(float3 rotation);
	void setTranslation(float3 translation);

	void createMesh(float* meshPos, float* meshNor, float* meshUV, unsigned long* meshInd, size_t numVert, size_t numInd);
	void createCube();
	void createTriangle();
	void createQuad();

private:
	ID3D12Device* device;
	VertexBuffer* pos;
	VertexBuffer* nor;
	VertexBuffer* uvs;
	VertexBuffer* ind;

	DirectX::XMFLOAT3 scale;
	DirectX::XMFLOAT4 rotationQuat;
	DirectX::XMFLOAT3 translation;
	DirectX::XMFLOAT4X4 worldMatrix;
};