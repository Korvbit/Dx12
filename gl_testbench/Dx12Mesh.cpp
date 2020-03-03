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
	for (int i = 0; i < nrOfKeyframes; i++)
	{
		delete pos[i];
		delete nor[i];
	}
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
	tmpMat *= DirectX::XMLoadFloat4x4(&((Dx12Camera*)camera)->vpMatrix);

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
	tmpVec = DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&this->rotationQuat), XMQuaternionRotationRollPitchYaw(rotate.x, rotate.y, rotate.z)));
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
	DirectX::XMStoreFloat4(&this->rotationQuat, XMQuaternionNormalize(XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z)));
}

void Dx12Mesh::setTranslation(float3 translation)
{
	this->translation = DirectX::XMFLOAT3(translation.x, translation.y, translation.z);
}

void Dx12Mesh::createMeshFromObj(const wchar_t * filepath, unsigned int keyframes)
{
	nrOfKeyframes = keyframes;

	WaveFrontReader<DWORD> objLoader;
	wchar_t fileNameBuffer[256];
	swprintf_s(fileNameBuffer, L"%s%d.obj", filepath, 1);
	objLoader.Load(fileNameBuffer);
	std::vector<DirectX::XMFLOAT4> objPos;
	std::vector<DirectX::XMFLOAT4> objNor;
	std::vector<DirectX::XMFLOAT2> objUV;
	std::vector<DWORD> objInd = objLoader.indices;

	for (auto work : objLoader.vertices)
	{
		objPos.push_back(DirectX::XMFLOAT4(work.position.x, work.position.y, work.position.z, 1.0f));
		objNor.push_back(DirectX::XMFLOAT4(work.normal.x, work.normal.y, work.normal.z, 0.0f));
		objUV.push_back(work.textureCoordinate);
	}

	pos.push_back(new Dx12VertexBuffer(sizeof(DirectX::XMFLOAT4) * objPos.size(), objPos.size(), device));
	nor.push_back(new Dx12VertexBuffer(sizeof(DirectX::XMFLOAT4) * objNor.size(), objNor.size(), device));
	uvs = new Dx12VertexBuffer(sizeof(DirectX::XMFLOAT2) * objUV.size(), objUV.size(), device);
	ind = new Dx12IndexBuffer(sizeof(DWORD) * objInd.size(), objInd.size(), device);

	pos[0]->setData(&objPos[0], sizeof(DirectX::XMFLOAT4) * objPos.size(), 0);
	nor[0]->setData(&objNor[0], sizeof(DirectX::XMFLOAT4) * objNor.size(), 0);
	uvs->setData(&objUV[0], sizeof(DirectX::XMFLOAT2) * objUV.size(), 0);
	ind->setData(&objInd[0], sizeof(DWORD) * objInd.size(), 0);

	geometryBuffers[POSITION] = { sizeof(DirectX::XMFLOAT4), objPos.size(), 0, pos[0] };
	geometryBuffers[NORMAL] = { sizeof(DirectX::XMFLOAT4), objNor.size(), 0, nor[0] };
	geometryBuffers[TEXCOORD] = { sizeof(DirectX::XMFLOAT2), objUV.size(), 0, uvs };
	geometryBuffers[INDEXBUFF] = { sizeof(DWORD), objInd.size(), 0, ind };

	for (int i = 1; i < keyframes; i++)
	{
		WaveFrontReader<DWORD> objLoader;
		wchar_t fileNameBuffer[256];
		swprintf_s(fileNameBuffer, L"%s%d.obj", filepath, i+1);
		objLoader.Load(fileNameBuffer);
		std::vector<DirectX::XMFLOAT4> objPos;
		std::vector<DirectX::XMFLOAT4> objNor;

		for (auto work : objLoader.vertices)
		{
			objPos.push_back(DirectX::XMFLOAT4(work.position.x, work.position.y, work.position.z, 1.0f));
			objNor.push_back(DirectX::XMFLOAT4(work.normal.x, work.normal.y, work.normal.z, 0.0f));
		}

		pos.push_back(new Dx12VertexBuffer(sizeof(DirectX::XMFLOAT4) * objPos.size(), objPos.size(), device));
		nor.push_back(new Dx12VertexBuffer(sizeof(DirectX::XMFLOAT4) * objNor.size(), objNor.size(), device));

		pos[i]->setData(&objPos[0], sizeof(DirectX::XMFLOAT4) * objPos.size(), 0);
		nor[i]->setData(&objNor[0], sizeof(DirectX::XMFLOAT4) * objNor.size(), 0);

		geometryBuffers[POSITION + i*2] = { sizeof(DirectX::XMFLOAT4), objPos.size(), 0, pos[i] };
		geometryBuffers[NORMAL + i*2] = { sizeof(DirectX::XMFLOAT4), objNor.size(), 0, nor[i] };
	}
}

void Dx12Mesh::createMesh(float* meshPos, float* meshNor, float* meshUV, DWORD * meshInd, size_t numVert, size_t numInd)
{
	nrOfKeyframes = 1;

	pos.push_back(new Dx12VertexBuffer(sizeof(float4) * numVert, numVert, device));
	nor.push_back(new Dx12VertexBuffer(sizeof(float4) * numVert, numVert, device));
	uvs = new Dx12VertexBuffer(sizeof(float2) * numVert, numVert, device);
	ind = new Dx12IndexBuffer(sizeof(DWORD) * numInd, numInd, device);

	pos[0]->setData(meshPos, sizeof(float4) * numVert, 0);
	nor[0]->setData(meshNor, sizeof(float4) * numVert, 0);
	uvs->setData(meshUV, sizeof(float2) * numVert, 0);
	ind->setData(meshInd, sizeof(DWORD) * numInd, 0);

	geometryBuffers[POSITION] = { sizeof(float4), numVert, 0, pos[0] };
	geometryBuffers[NORMAL] = { sizeof(float4), numVert, 0, nor[0] };
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
