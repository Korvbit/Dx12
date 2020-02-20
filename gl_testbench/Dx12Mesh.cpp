#include "Dx12Mesh.h"

Dx12Mesh::Dx12Mesh(float4 scale, float4 rotate, float4 translate)
{
	DirectX::XMFLOAT4 tmpFloat4;
	DirectX::XMVECTOR tmpVec;
	DirectX::XMMATRIX tmpMat;

	tmpFloat4 = DirectX::XMFLOAT4(scale.x, scale.y, scale.z, scale.w);
	tmpVec = DirectX::XMLoadFloat4(&tmpFloat4);
	tmpMat = DirectX::XMMatrixScalingFromVector(tmpVec);
	DirectX::XMStoreFloat4x4(&scaleMatrix, tmpMat);

	DirectX::XMMATRIX rotMatX = DirectX::XMMatrixRotationX(rotate.x);
	DirectX::XMMATRIX rotMatY = DirectX::XMMatrixRotationY(rotate.y);
	DirectX::XMMATRIX rotMatZ = DirectX::XMMatrixRotationZ(rotate.z);
	tmpMat = rotMatX * rotMatY * rotMatZ;
	DirectX::XMStoreFloat4x4(&rotationMatrix, tmpMat);

	tmpFloat4 = DirectX::XMFLOAT4(translate.x, translate.y, translate.z, translate.w);
	tmpVec = DirectX::XMLoadFloat4(&tmpFloat4);
	tmpMat = DirectX::XMMatrixTranslationFromVector(tmpVec);
	DirectX::XMStoreFloat4x4(&translationMatrix, tmpMat);
}

Dx12Mesh::~Dx12Mesh()
{
}

void Dx12Mesh::Update(float4 translate, float4 rotate, float4 scale)
{

}

void Dx12Mesh::scale(float4 scale, bool linear)
{
	DirectX::XMMATRIX scaleMat = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	DirectX::XMMATRIX originalScale = DirectX::XMLoadFloat4x4(&scaleMatrix);
	if (linear)
		DirectX::XMStoreFloat4x4(&scaleMatrix, originalScale + scaleMat);
	else
		DirectX::XMStoreFloat4x4(&scaleMatrix, originalScale * scaleMat);
}

void Dx12Mesh::rotate(float4 rotate)
{
	DirectX::XMMATRIX rotMatX = DirectX::XMMatrixRotationX(rotate.x);
	DirectX::XMMATRIX rotMatY = DirectX::XMMatrixRotationY(rotate.y);
	DirectX::XMMATRIX rotMatZ = DirectX::XMMatrixRotationZ(rotate.z);

	DirectX::XMMATRIX originalRot = DirectX::XMLoadFloat4x4(&rotationMatrix);
	DirectX::XMMATRIX rotMat = originalRot * rotMatX * rotMatY * rotMatZ;

	DirectX::XMStoreFloat4x4(&rotationMatrix, rotMat);
}

void Dx12Mesh::translate(float4 translate)
{
	DirectX::XMMATRIX transMat = DirectX::XMMatrixTranslation(translate.x, translate.y, translate.z);
	DirectX::XMMATRIX originalTrans = DirectX::XMLoadFloat4x4(&translationMatrix);

	DirectX::XMStoreFloat4x4(&translationMatrix, originalTrans + transMat);

}

void Dx12Mesh::createCube()
{
}

void Dx12Mesh::createTriangle()
{
	float4 triPos[3] = { { 0.0f,  0.05, 0.0f, 1.0f },{ 0.05, -0.05, 0.0f, 1.0f },{ -0.05, -0.05, 0.0f, 1.0f } };
	float4 triNor[3] = { { 0.0f,  0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f, 0.0f } };
	float2 triUV[3] = { { 0.5f,  -0.99f },{ 1.49f, 1.1f },{ -0.51, 1.1f } };
}

void Dx12Mesh::createQuad()
{
}
