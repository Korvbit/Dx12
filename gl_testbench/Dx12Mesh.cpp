#include "Dx12Mesh.h"

Dx12Mesh::Dx12Mesh()
{
}

Dx12Mesh::Dx12Mesh(ID3D12Device* rendererDevice)
{
	device = rendererDevice;
}

Dx12Mesh::~Dx12Mesh()
{
	delete pos;
	delete nor;
	delete uvs;

	delete technique;

	// translation buffers
	delete txBuffer;
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

void Dx12Mesh::createCube()
{
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

	pos = new Dx12VertexBuffer(sizeof(triPos), std::extent<decltype(triPos)>::value, device);
	nor = new Dx12VertexBuffer(sizeof(triNor), std::extent<decltype(triNor)>::value, device);
	uvs = new Dx12VertexBuffer(sizeof(triUV), std::extent<decltype(triUV)>::value, device);

	pos->setData(triPos, sizeof(triPos), 0);
	nor->setData(triNor, sizeof(triNor), 0);
	uvs->setData(triUV, sizeof(triUV), 0);

	geometryBuffers[POSITION] = { sizeof(float4), 3, 0, pos };
	geometryBuffers[NORMAL] = { sizeof(float4), 3, 0, nor };
	geometryBuffers[TEXCOORD] = { sizeof(float2), 3, 0, uvs };
}

void Dx12Mesh::createQuad()
{
	float4 quadPos[6] = {	{  0.05f,  0.05f, 0.0f, 1.0f },
							{  0.05f, -0.05f, 0.0f, 1.0f },
							{ -0.05f, -0.05f, 0.0f, 1.0f },

							{  0.05f,  0.05f, 0.0f, 1.0f },
							{ -0.05f, -0.05f, 0.0f, 1.0f },
							{ -0.05f,  0.05f, 0.0f, 1.0f }
	};
	float4 quadNor[6] = {	{ 0.0f, 0.0f, 1.0f, 0.0f },
							{ 0.0f, 0.0f, 1.0f, 0.0f },
							{ 0.0f, 0.0f, 1.0f, 0.0f },

							{ 0.0f, 0.0f, 1.0f, 0.0f },
							{ 0.0f, 0.0f, 1.0f, 0.0f },
							{ 0.0f, 0.0f, 1.0f, 0.0f }
	};
	float2 quadUV[6] = {	{  1.00f, -1.00f },
							{  1.00f,  0.00f },
							{  0.00f,  0.00f },

							{  1.00f, -1.00f },
							{  0.00f,  0.00f },
							{  0.00f, -1.00f },
	};

	DWORD indexList[] = {
		0, 1, 2,
		0, 3, 1
	};


	pos = new Dx12VertexBuffer(sizeof(quadPos), 6, device);
	nor = new Dx12VertexBuffer(sizeof(quadNor), 6, device);
	uvs = new Dx12VertexBuffer(sizeof(quadUV), 6, device);

	pos->setData(quadPos, sizeof(quadPos), 0);
	nor->setData(quadNor, sizeof(quadNor), 0);
	uvs->setData(quadUV, sizeof(quadUV), 0);

	geometryBuffers[POSITION] = { sizeof(float4), 6, 0, pos };
	geometryBuffers[NORMAL] = { sizeof(float4), 6, 0, nor };
	geometryBuffers[TEXCOORD] = { sizeof(float2), 6, 0, uvs };
}
