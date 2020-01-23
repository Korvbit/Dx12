#include "Dx12Renderer.h"



Dx12Renderer::Dx12Renderer()
{
}


Dx12Renderer::~Dx12Renderer()
{
}

int Dx12Renderer::initialize(unsigned int width, unsigned int height)
{
	// Find compatible adapter
	IDXGIFactory5* factory = nullptr;
	IDXGIAdapter1* adapter = nullptr;

	CreateDXGIFactory(IID_PPV_ARGS(&factory));
	for (UINT adapterIndex = 0;; ++adapterIndex) {
		adapter = nullptr;
		if (DXGI_ERROR_NOT_FOUND == factory->EnumAdapters1(adapterIndex, &adapter)) {
			break;
		}

		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device), nullptr))) {
			break;
		}

		SafeRelease(&adapter);
	}

	// Create Device
	if (adapter) {
		D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device));

		SafeRelease(&adapter);
	}
	else {	// If we did not find a compatible device we create a "warp device"
		factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
		D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
	}

	// Create the command queue
	D3D12_COMMAND_QUEUE_DESC cqd = {};
	device->CreateCommandQueue(&cqd, IID_PPV_ARGS(&commandQueue));

	// Create command allocators
	//for (int i = 0; i < frameBufferCount; ++i)
	//	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[i]));
	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[0]));

	// Create command lists
	device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandAllocator[0],
		nullptr,
		IID_PPV_ARGS(&commandList));

	commandList->Close();

	// Create fences
	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence[0]));
	fenceValue[0] = 1;
	fenceEvent = CreateEvent(0, false, false, 0);

	//SafeRelease(&factory);
	//factory = nullptr;
	//CreateDXGIFactory(IID_PPV_ARGS(&factory));

	//IDXGISwapChain1* swapChain1 = nullptr;
	//
	//DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	//scDesc.Width = 0;
	//scDesc.Height = 0;
	//scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//scDesc.Stereo = FALSE;
	//scDesc.SampleDesc.Count = 1;
	//scDesc.SampleDesc.Quality = 0;
	//scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	//scDesc.BufferCount = frameBufferCount;
	//scDesc.Scaling = DXGI_SCALING_NONE;
	//scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	//scDesc.Flags = 0;
	//scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	//
	//HRESULT hr = factory->CreateSwapChainForHwnd(
	//	commandQueue,
	//	wndHandle,
	//	&scDesc,
	//	nullptr,
	//	nullptr,
	//	reinterpret_cast<IDXGISwapChain1**>(&swapChain)
	//);

	SafeRelease(&factory);

	return 0;
}

void Dx12Renderer::WaitForGpu()
{
	// Signal when the fence has increased in value
	const UINT64 signalValue = fenceValue[0];
	commandQueue->Signal(fence[0], signalValue);
	fenceValue[0]++; // Increment the comparison value inbefore the next call

	// Wait until the value has been incremented
	if (fence[0]->GetCompletedValue() < signalValue) {
		fence[0]->SetEventOnCompletion(signalValue, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
	}
}
