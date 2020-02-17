#pragma once
#include <unordered_map>
#include "Mesh.h"
#include "Dx12/functions.h"

class Dx12Mesh :
	public Mesh
{
public:
	Dx12Mesh();
	~Dx12Mesh();

	void createCube();
	void createTriangle();
	void createQuad();
};