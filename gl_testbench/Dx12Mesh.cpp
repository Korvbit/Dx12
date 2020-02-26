#include "Dx12Mesh.h"

using namespace DirectX;

Dx12Mesh::Dx12Mesh(ID3D12Device* rendererDevice, float3 translate, float3 rotate, float3 scale)
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

	// translation buffers
	delete wvpBuffer;
}

void Dx12Mesh::Update(Camera* camera)
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
	DirectX::XMStoreFloat4x4(&worldMatrix, tmpMat);

	// Create wvpMatrix
	tmpMat *= DirectX::XMLoadFloat4x4(&((Dx12Camera*)camera)->viewMatrix) * DirectX::XMLoadFloat4x4(&((Dx12Camera*)camera)->projectionMatrix);

	DirectX::XMFLOAT4X4 wvpMatrix;
	DirectX::XMStoreFloat4x4(&wvpMatrix, DirectX::XMMatrixTranspose(tmpMat));

	// Upload wvpMatrix
	wvpBuffer->setData(&wvpMatrix, sizeof(wvpMatrix), technique->getMaterial(), TRANSLATION);
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

void Dx12Mesh::createMesh(float* meshPos, float* meshNor, float* meshUV, DWORD * meshInd, size_t numVert, size_t numInd)
{
	pos = new Dx12VertexBuffer(sizeof(float4) * numVert, numVert, device);
	nor = new Dx12VertexBuffer(sizeof(float4) * numVert, numVert, device);
	uvs = new Dx12VertexBuffer(sizeof(float2) * numVert, numVert, device);
	ind = new Dx12IndexBuffer(sizeof(DWORD) * numInd, numInd, device);

	pos->setData(meshPos, sizeof(float4) * numVert, 0);
	nor->setData(meshNor, sizeof(float4) * numVert, 0);
	uvs->setData(meshUV, sizeof(float2) * numVert, 0);
	ind->setData(meshInd, sizeof(DWORD) * numInd, 0);

	geometryBuffers[POSITION] = { sizeof(float4), numVert, 0, pos };
	geometryBuffers[NORMAL] = { sizeof(float4), numVert, 0, nor };
	geometryBuffers[TEXCOORD] = { sizeof(float2), numVert, 0, uvs };
	geometryBuffers[INDEXBUFF] = { sizeof(DWORD), numInd, 0, ind };
}

void Dx12Mesh::createCube()
{
	float cubePos[] = {
							 0.05f,  0.05f, -0.05f, 1.0f,
							 0.05f, -0.05f, -0.05f, 1.0f,
							-0.05f, -0.05f, -0.05f, 1.0f,
							-0.05f,  0.05f, -0.05f, 1.0f,
							 0.05f,  0.05f,  0.05f, 1.0f,
							 0.05f, -0.05f,  0.05f, 1.0f,
							-0.05f, -0.05f,  0.05f, 1.0f,
							-0.05f,  0.05f,  0.05f, 1.0f,
	};
	float cubeNor[] = {
							0.0f, 0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 1.0f, 0.0f,
	};
	float cubeUV[] = {
							1.00f, -1.00f,
							1.00f,  0.00f,
							0.00f,  0.00f,
							0.00f, -1.00f,
							1.00f, -1.00f,
							1.00f,  0.00f,
							0.00f,  0.00f,
							0.00f, -1.00f,
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

	createMesh(cubePos, cubeNor, cubeUV, indexList, 8, ARRAYSIZE(indexList));
}

void Dx12Mesh::createTriangle()
{
	float triPos[] = {
						 0.00f,  0.05f, 0.0f, 1.0f,
						 0.05f, -0.05f, 0.0f, 1.0f,
						-0.05f, -0.05f, 0.0f, 1.0f
	};
	float triNor[] = {
						0.0f,  0.0f, 1.0f, 0.0f,
						0.0f,  0.0f, 1.0f, 0.0f,
						0.0f,  0.0f, 1.0f, 0.0f
	};
	float triUV[] = {
							 0.50f, -0.99f,
							 1.49f,  1.10f,
							-0.51f,  1.10f
	};

	DWORD indexList[] = {
		0, 1, 2
	};

	createMesh(triPos, triNor, triUV, indexList, 3, ARRAYSIZE(indexList));
}

void Dx12Mesh::createQuad()
{
	float quadPos[] = {
							 0.05f,  0.05f, 0.0f, 1.0f,
							 0.05f, -0.05f, 0.0f, 1.0f,
							-0.05f, -0.05f, 0.0f, 1.0f,
							-0.05f,  0.05f, 0.0f, 1.0f,
	};
	float quadNor[] = {
							0.0f, 0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 1.0f, 0.0f,
	};
	float quadUV[] = {
							1.00f, -1.00f,
							1.00f,  0.00f,
							0.00f,  0.00f,
							0.00f, -1.00f,
	};

	DWORD indexList[] = {
		0, 1, 2,
		0, 2, 3
	};

	createMesh(quadPos, quadNor, quadUV, indexList, 4, ARRAYSIZE(indexList));
}
