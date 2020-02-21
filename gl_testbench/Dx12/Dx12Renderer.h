#pragma once
#include "../Renderer.h"

#include <d3d12.h>
#include <dxgi1_5.h>
#include <D3Dcompiler.h>
#include "Dx12Texture2D.h"
#include "..//Dx12Material.h"
#include "..//Dx12RenderState.h"
#include "..//Dx12Sampler2D.h"
#include "..//Dx12VertexBuffer.h"
#include "..//Dx12Mesh.h"
#include "d3dx12.h"
#include "functions.h"
#include "..//Dx12Camera.h"

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

	ID3D12Device* device;
	ID3D12DescriptorHeap* rtvDescriptorHeap;
	ID3D12DescriptorHeap* dsDescriptorHeap;
	int rtvDescriptorSize;
	ID3D12Resource* renderTargets[frameBufferCount];
	ID3D12Resource* depthStencilBuffer;
	ID3D12CommandQueue* commandQueue;
	ID3D12CommandAllocator* commandAllocator[frameBufferCount];
	ID3D12GraphicsCommandList* commandList;
	ID3D12Fence* fence[frameBufferCount];
	UINT64 fenceValue[frameBufferCount];
	HANDLE fenceEvent;
	IDXGISwapChain3* swapChain;
	std::unordered_map<Technique*, std::vector<Mesh*>> drawList;
	unsigned int clearFlags;

	Camera* makeCamera(unsigned int width, unsigned int height);
	Material* makeMaterial(const std::string& name);
	Mesh* makeMesh();
	VertexBuffer* makeVertexBuffer(size_t size, VertexBuffer::DATA_USAGE usage);
	Texture2D* makeTexture2D();
	Sampler2D* makeSampler2D();
	RenderState* makeRenderState();
	ConstantBuffer* makeConstantBuffer(std::string NAME, unsigned int location);
	Technique* makeTechnique(Material*, RenderState*);
	void setResourceTransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource,
		D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter);

	int initialize(unsigned int width, unsigned int height);
	void setWinTitle(const char* title);
	void present();	// Swap buffers
	int shutdown() { return -1;	};

	void setClearColor(float r, float g, float b, float a);
	void clearBuffer(unsigned int flags);
	void submit(Mesh* mesh);
	void frame();

	bool initializeWindow(HINSTANCE hInstance, int width, int height, bool fullscreen);
	void WaitForGpu();
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);