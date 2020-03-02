#pragma once
#include <unordered_map>
#include "IA.h"
#include "VertexBuffer.h"
#include "Technique.h"
#include "Transform.h"
#include "ConstantBuffer.h"
#include "Texture2D.h"
#include "Dx12/functions.h"
#include "Camera.h"

class Mesh
{
public:
	Mesh();
	virtual ~Mesh();

	virtual void createMeshFromObj(const wchar_t* filename) = 0;
	virtual void createMesh(float* meshPos, float* meshNor, float* meshUV, unsigned long* meshInd, size_t numVert, size_t numInd) = 0;
	virtual void createCube() = 0;
	virtual void createTriangle() = 0;
	virtual void createQuad() = 0;

	// technique has: Material, RenderState, Attachments (color, depth, etc)
	Technique* technique; 

	// translation buffers
	ConstantBuffer* wvpBuffer;
	// local copy of the translation
	Transform* transform;

	struct VertexBufferBind {
		size_t sizeElement, numElements, offset;
		VertexBuffer* buffer;
	};
	
	void addTexture(Texture2D* texture, unsigned int slot);

	// array of buffers with locations (binding points in shaders)
	void addIAVertexBufferBinding(
		VertexBuffer* buffer, 
		size_t offset, 
		size_t numElements, 
		size_t sizeElement, 
		unsigned int inputStream);

	std::unordered_map<unsigned int, VertexBufferBind> geometryBuffers;
	std::unordered_map<unsigned int, Texture2D*> textures;

	virtual void Update(Camera* camera) = 0;

	virtual void scaleMesh(float3 scale) = 0;
	virtual void rotateMesh(float3 rotate) = 0;
	virtual void translateMesh(float3 translate) = 0;

	virtual void setScale(float3 scale) = 0;
	virtual void setRotation(float3 rotation) = 0;
	virtual void setTranslation(float3 translation) = 0;

};
