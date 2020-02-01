#pragma once
#include "../Renderer.h"

#include <d3d12.h>
#include <dxgi1_5.h>
#include <D3Dcompiler.h>
#include "Dx12Texture2D.h"
#include "d3dx12.h"
#include "functions.h"

struct Vertex {
	float pos[3];
	float texCoords[2];
};

struct CBtranslate {
	float translate[4];
};

class Dx12Renderer :
	public Renderer
{
public:
	Dx12Renderer();
	~Dx12Renderer();

	static const int frameBufferCount = 3; // Currently tripple buffering.
	int frameIndex; // Current rtv
	float clearColor[4];
	int direction = 1;

	HWND hwnd = NULL;
	LPCTSTR WindowName = L"temp";
	LPCTSTR WindowTitle = L"temp";

	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;

	ID3D12RootSignature* rootSignature;
	ID3D12PipelineState* pipelineStateObject;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

	ID3D12Device* device;
	ID3D12DescriptorHeap* descriptorHeap[frameBufferCount];
	ID3D12DescriptorHeap* rtvDescriptorHeap;
	ID3D12DescriptorHeap* dsDescriptorHeap;
	int rtvDescriptorSize;
	ID3D12Resource* textureBufferUploadHeap;
	ID3D12Resource1* constantBufferResource[frameBufferCount];
	ID3D12Resource* renderTargets[frameBufferCount];
	ID3D12Resource* textureBuffer;
	ID3D12Resource* vertexBuffer;
	ID3D12Resource* depthStencilBuffer;
	CBtranslate translationBuffer;
	ID3D12CommandQueue* commandQueue;
	ID3D12CommandAllocator* commandAllocator[frameBufferCount];
	ID3D12GraphicsCommandList* commandList;
	ID3D12Fence* fence[frameBufferCount];
	UINT64 fenceValue[frameBufferCount];
	HANDLE fenceEvent;
	IDXGISwapChain3* swapChain;
	UINT8* cbvGPUAddress[frameBufferCount];


	Material* makeMaterial(const std::string& name) { return nullptr; };
	Mesh* makeMesh() { return nullptr; };
	VertexBuffer* makeVertexBuffer(size_t size, VertexBuffer::DATA_USAGE usage) { return nullptr; };
	Texture2D* makeTexture2D();
	Sampler2D* makeSampler2D() { return nullptr; };
	RenderState* makeRenderState() { return nullptr; };
	std::string getShaderPath() { return nullptr; };
	std::string getShaderExtension() { return nullptr; };
	ConstantBuffer* makeConstantBuffer(std::string NAME, unsigned int location) { return nullptr; };
	Technique* makeTechnique(Material*, RenderState*) { return nullptr; };
	void setResourceTransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource,
		D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter);

	int initialize(unsigned int width = 800, unsigned int height = 600);
	void setWinTitle(const char* title);
	void present() {};	// Swap buffers
	int shutdown() { return -1;	};

	void setClearColor(float r, float g, float b, float a);
	void clearBuffer(unsigned int) {};
	void setRenderState(RenderState* ps) {};
	void submit(Mesh* mesh) {};
	void frame();

	bool initializeWindow(HINSTANCE hInstance, int width, int height, bool fullscreen);
	void WaitForGpu();
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);