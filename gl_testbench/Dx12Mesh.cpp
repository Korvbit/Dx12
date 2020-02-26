#include "Dx12Mesh.h"

using namespace DirectX;

Dx12Mesh::Dx12Mesh(ID3D12Device* rendererDevice, float3 scale, float3 rotate, float3 translate)
{
	device = rendererDevice;

	DirectX::XMFLOAT3 tmpFloat4;
	DirectX::XMVECTOR tmpVec;

	// Store and apply scale
	tmpFloat4 = DirectX::XMFLOAT3(scale.x, scale.y, scale.z);
	tmpVec = DirectX::XMLoadFloat3(&tmpFloat4);
	DirectX::XMStoreFloat3(&this->scale, tmpVec);

	// Store and apply rotation quaternion
	tmpVec = XMQuaternionRotationRollPitchYaw(rotate.z, rotate.x, rotate.y);
	DirectX::XMStoreFloat4(&this->rotationQuat, tmpVec);

	//Store and apply translation
	tmpFloat4 = DirectX::XMFLOAT3(translate.x, translate.y, translate.z);
	tmpVec = DirectX::XMLoadFloat3(&tmpFloat4);
	DirectX::XMStoreFloat3(&this->translation, tmpVec);
}

Dx12Mesh::~Dx12Mesh()
{
	delete pos;
	delete nor;
	delete uvs;
	delete ind;

	delete technique;

	// translation buffers
	delete wvpBuffer;
	// local copy of the translation
	delete transform;

	for (auto work : geometryBuffers)
	{
		delete work.second.buffer;
	}

	for (auto work : textures)
	{
		delete work.second;
	}
}

void Dx12Mesh::Update()
{
	// Create the world matrix
	DirectX::XMVECTOR tmpVec;
	DirectX::XMMATRIX tmpMat;

	// Scale
	tmpVec = DirectX::XMLoadFloat3(&this->scale);
	tmpMat = DirectX::XMMatrixScalingFromVector(tmpVec);
	// Rotate
	tmpVec = DirectX::XMLoadFloat4(&this->rotationQuat);
	tmpMat *= DirectX::XMMatrixRotationQuaternion(tmpVec);
	// Translate
	tmpVec = DirectX::XMLoadFloat3(&this->translation);
	tmpMat *= DirectX::XMMatrixTranslationFromVector(tmpVec);

	// Store the world matrix
	DirectX::XMStoreFloat4x4(&worldMatrix, DirectX::XMMatrixTranspose(tmpMat));

	wvpBuffer->setData(&this->worldMatrix, sizeof(this->worldMatrix), technique->getMaterial(), TRANSLATION);
}

void Dx12Mesh::scaleMesh(float3 scale)
{
	this->scale = 
		DirectX::XMFLOAT3(	this->scale.x + scale.x,
							this->scale.y + scale.y,
							this->scale.z + scale.z);
}

void Dx12Mesh::rotateMesh(float3 rotate)
{
	DirectX::XMVECTOR tmpVec;
	tmpVec = DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&this->rotationQuat), XMQuaternionRotationRollPitchYaw(rotate.z, rotate.x, rotate.y));
	DirectX::XMStoreFloat4(&this->rotationQuat, tmpVec);
}

void Dx12Mesh::translateMesh(float3 translate)
{
	this->translation = 
		DirectX::XMFLOAT3(	this->translation.x + translate.x,
							this->translation.y + translate.y,
							this->translation.z + translate.z);
}

void Dx12Mesh::setScale(float3 scale)
{
	this->scale = DirectX::XMFLOAT3(scale.x, scale.y, scale.z);
}

void Dx12Mesh::setRotation(float3 rotation)
{
	DirectX::XMStoreFloat4(&this->rotationQuat, XMQuaternionRotationRollPitchYaw(rotation.z, rotation.x, rotation.y));
}

void Dx12Mesh::setTranslation(float3 translation)
{
	this->translation = DirectX::XMFLOAT3(translation.x, translation.y, translation.z);
}

void Dx12Mesh::createCube()
{
	float4 cubePos[8] = {
							{  0.05f,  0.05f, 0.0f, 1.0f },
							{  0.05f, -0.05f, 0.0f, 1.0f },
							{ -0.05f, -0.05f, 0.0f, 1.0f },
							{ -0.05f,  0.05f, 0.0f, 1.0f },
							{  0.05f,  0.05f, 0.1f, 1.0f },
							{  0.05f, -0.05f, 0.1f, 1.0f },
							{ -0.05f, -0.05f, 0.1f, 1.0f },
							{ -0.05f,  0.05f, 0.1f, 1.0f },
	};
	float4 cubeNor[8] = {
							{ 0.0f, 0.0f, 1.0f, 0.0f },
							{ 0.0f, 0.0f, 1.0f, 0.0f },
							{ 0.0f, 0.0f, 1.0f, 0.0f },
							{ 0.0f, 0.0f, 1.0f, 0.0f },
							{ 0.0f, 0.0f, 1.0f, 0.0f },
							{ 0.0f, 0.0f, 1.0f, 0.0f },
							{ 0.0f, 0.0f, 1.0f, 0.0f },
							{ 0.0f, 0.0f, 1.0f, 0.0f },
	};
	float2 cubeUV[8] = {
							{  1.00f, -1.00f },
							{  1.00f,  0.00f },
							{  0.00f,  0.00f },
							{  0.00f, -1.00f },
							{  1.00f, -1.00f },
							{  1.00f,  0.00f },
							{  0.00f,  0.00f },
							{  0.00f, -1.00f },
	};

	DWORD indexList[] = {
		// Front face
		0, 1, 2,
		0, 2, 3,
		
		// Back face
		7, 6, 5,
		7, 5, 4,

		// Right face
		4, 5, 1,
		4, 1, 0,

		// Left face
		3, 2, 6,
		3, 6, 7,

		// Top face
		4, 0, 3,
		4, 3, 7,

		// Bottom face
		1, 5, 6,
		1, 6, 2,
	};


	pos = new Dx12VertexBuffer(sizeof(cubePos), std::extent<decltype(cubePos)>::value, device);
	nor = new Dx12VertexBuffer(sizeof(cubeNor), std::extent<decltype(cubeNor)>::value, device);
	uvs = new Dx12VertexBuffer(sizeof(cubeUV), std::extent<decltype(cubeUV)>::value, device);
	ind = new Dx12IndexBuffer(sizeof(indexList), ARRAYSIZE(indexList), device);

	pos->setData(cubePos, sizeof(cubePos), 0);
	nor->setData(cubeNor, sizeof(cubeNor), 0);
	uvs->setData(cubeUV, sizeof(cubeUV), 0);
	ind->setData(indexList, sizeof(indexList), 0);

	geometryBuffers[POSITION] = { sizeof(float4), std::extent<decltype(cubePos)>::value, 0, pos };
	geometryBuffers[NORMAL] = { sizeof(float4), std::extent<decltype(cubeNor)>::value, 0, nor };
	geometryBuffers[TEXCOORD] = { sizeof(float2), std::extent<decltype(cubeUV)>::value, 0, uvs };
	geometryBuffers[INDEXBUFF] = { sizeof(DWORD), ARRAYSIZE(indexList), 0, ind };
}

void Dx12Mesh::createTriangle()
{
	float4 triPos[3] = {	{  0.00f,  0.05f, 0.0f, 1.0f },
							{  0.05f, -0.05f, 0.0f, 1.0f },
							{ -0.05f, -0.05f, 0.0f, 1.0f }
	};
	float4 triNor[3] = {	{ 0.0f,  0.0f, 1.0f, 0.0f },
							{ 0.0f,  0.0f, 1.0f, 0.0f },
							{ 0.0f,  0.0f, 1.0f, 0.0f }
	};
	float2 triUV[3] = {		{  0.50f, -0.99f },
							{  1.49f,  1.10f },
							{ -0.51f,  1.10f }
	};

	DWORD indexList[] = {
		0, 1, 2
	};

	pos = new Dx12VertexBuffer(sizeof(triPos), std::extent<decltype(triPos)>::value, device);
	nor = new Dx12VertexBuffer(sizeof(triNor), std::extent<decltype(triNor)>::value, device);
	uvs = new Dx12VertexBuffer(sizeof(triUV), std::extent<decltype(triUV)>::value, device);
	ind = new Dx12IndexBuffer(sizeof(indexList), 3, device);

	pos->setData(triPos, sizeof(triPos), 0);
	nor->setData(triNor, sizeof(triNor), 0);
	uvs->setData(triUV, sizeof(triUV), 0);
	ind->setData(indexList, sizeof(indexList), 0);

	geometryBuffers[POSITION] = { sizeof(float4), std::extent<decltype(triPos)>::value, 0, pos };
	geometryBuffers[NORMAL] = { sizeof(float4), std::extent<decltype(triNor)>::value, 0, nor };
	geometryBuffers[TEXCOORD] = { sizeof(float2), std::extent<decltype(triUV)>::value, 0, uvs };
	geometryBuffers[INDEXBUFF] = { sizeof(DWORD), ARRAYSIZE(indexList), 0, ind };
}

void Dx12Mesh::createQuad()
{
	float4 quadPos[4] = {
							{  0.05f,  0.05f, 0.0f, 1.0f },
							{  0.05f, -0.05f, 0.0f, 1.0f },
							{ -0.05f, -0.05f, 0.0f, 1.0f },
							{ -0.05f,  0.05f, 0.0f, 1.0f },
	};
	float4 quadNor[4] = {
							{ 0.0f, 0.0f, 1.0f, 0.0f },
							{ 0.0f, 0.0f, 1.0f, 0.0f },
							{ 0.0f, 0.0f, 1.0f, 0.0f },
							{ 0.0f, 0.0f, 1.0f, 0.0f },
	};
	float2 quadUV[4] = {
							{  1.00f, -1.00f },
							{  1.00f,  0.00f },
							{  0.00f,  0.00f },
							{  0.00f, -1.00f },
	};

	DWORD indexList[] = {
		0, 1, 2,
		0, 2, 3
	};


	pos = new Dx12VertexBuffer(sizeof(quadPos), std::extent<decltype(quadPos)>::value, device);
	nor = new Dx12VertexBuffer(sizeof(quadNor), std::extent<decltype(quadNor)>::value, device);
	uvs = new Dx12VertexBuffer(sizeof(quadUV), std::extent<decltype(quadUV)>::value, device);
	ind = new Dx12IndexBuffer(sizeof(indexList), ARRAYSIZE(indexList), device);

	pos->setData(quadPos, sizeof(quadPos), 0);
	nor->setData(quadNor, sizeof(quadNor), 0);
	uvs->setData(quadUV, sizeof(quadUV), 0);
	ind->setData(indexList, sizeof(indexList), 0);

	geometryBuffers[POSITION] = { sizeof(float4), std::extent<decltype(quadPos)>::value, 0, pos };
	geometryBuffers[NORMAL] = { sizeof(float4), std::extent<decltype(quadNor)>::value, 0, nor };
	geometryBuffers[TEXCOORD] = { sizeof(float2), std::extent<decltype(quadUV)>::value, 0, uvs };
	geometryBuffers[INDEXBUFF] = { sizeof(DWORD), ARRAYSIZE(indexList), 0, ind };
}
