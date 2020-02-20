#pragma once
#include <unordered_map>
#include "Mesh.h"
#include "Dx12/functions.h"
#include "Camera.h"
#include <DirectXMath.h>

class Dx12Mesh :
	public Mesh
{
public:
	Dx12Mesh(	float4 translate =	{ 0.0f, 0.0f, 0.0f, 0.0f },
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

	DirectX::XMFLOAT4X4 scaleMatrix;
	DirectX::XMFLOAT4X4 rotationMatrix;
	DirectX::XMFLOAT4X4 translationMatrix;
};