#pragma once
#include "../Renderer.h"

#include <d3d12.h>
#include <dxgi1_5.h>
#include <D3Dcompiler.h>
#include "d3dx12.h"
#include "functions.h"

class Dx12Renderer :
	public Renderer
{
public:
	Dx12Renderer();
	~Dx12Renderer();

	//=====================================================

	Material* makeMaterial(const std::string& name) { return nullptr; };
	Mesh* makeMesh() { return nullptr; };
	VertexBuffer* makeVertexBuffer(size_t size, VertexBuffer::DATA_USAGE usage) { return nullptr; };
	Texture2D* makeTexture2D() { return nullptr; };
	Sampler2D* makeSampler2D() { return nullptr; };
	RenderState* makeRenderState() { return nullptr; };
	std::string getShaderPath() { return nullptr; };
	std::string getShaderExtension() { return nullptr; };
	ConstantBuffer* makeConstantBuffer(std::string NAME, unsigned int location) { return nullptr; };
	Technique* makeTechnique(Material*, RenderState*) { return nullptr; };

	int initialize(unsigned int width = 800, unsigned int height = 600);
	void setWinTitle(const char* title) {};
	void present() {};	// Render
	int shutdown() { return -1;	};

	void setClearColor(float, float, float, float) {};
	void clearBuffer(unsigned int) {};
	// can be partially overriden by a specific Technique.
	void setRenderState(RenderState* ps) {};
	// submit work (to render) to the renderer.
	void submit(Mesh* mesh) {};
	void frame() {};

	//=====================================================

	static const int frameBufferCount = 3; // Currently tripple buffering.
	int frameIndex; // Current rtv

	ID3D12Device* device;
	ID3D12CommandQueue* commandQueue;
	ID3D12CommandAllocator* commandAllocator[frameBufferCount];
	ID3D12GraphicsCommandList* commandList;
	ID3D12Fence* fence[frameBufferCount];
	UINT64 fenceValue[frameBufferCount];
	HANDLE fenceEvent;
	IDXGISwapChain3* swapChain;
	ID3D12Resource* renderTargets[frameBufferCount];
	ID3D12DescriptorHeap* rtvDescriptorHeap;
	int rtvDescriptorSize;

	void WaitForGpu();
};

