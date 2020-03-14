#include "Dx12Renderer.h"

Dx12Renderer::Dx12Renderer()
{
	rootSignature = nullptr;

	device = nullptr;
	rtvDescriptorHeap = nullptr;
	dsDescriptorHeap = nullptr;
	depthStencilBuffer = nullptr;
	commandQueue = nullptr;
	swapChain = nullptr;
	fence = nullptr;

	for (int i = 0; i < frameBufferCount; i++)
		renderTargets[i] = nullptr;

	for (int i = 0; i < frameBufferCount * numThreads; i++) {
		commandList[i] = nullptr;
		commandAllocator[i] = nullptr;
	}
}

Dx12Renderer::~Dx12Renderer()
{
	shutdown();
}

Camera * Dx12Renderer::makeCamera(unsigned int width, unsigned int height)
{
	return new Dx12Camera(width, height);
}

Material * Dx12Renderer::makeMaterial()
{
	return new Dx12Material(device);
}

Mesh * Dx12Renderer::makeMesh()
{
	return new Dx12Mesh(device);
}

VertexBuffer * Dx12Renderer::makeVertexBuffer(size_t size, int numEntries)
{
	return new Dx12VertexBuffer(size, numEntries, device);
}

Texture2D * Dx12Renderer::makeTexture2D()
{
	frameIndex = swapChain->GetCurrentBackBufferIndex();
	return (Texture2D*)new Dx12Texture2D(device, commandList[frameIndex], commandQueue, commandAllocator[frameIndex]);
}

Sampler2D * Dx12Renderer::makeSampler2D()
{
	return new Dx12Sampler2D(device);
}

RenderState * Dx12Renderer::makeRenderState()
{
	RenderState* newRS = new Dx12RenderState(device);
	newRS->setWireFrame(false);
	return newRS;
}

ConstantBuffer * Dx12Renderer::makeConstantBuffer(unsigned int location)
{
	return new Dx12ConstantBuffer(location, device);
}

Technique * Dx12Renderer::makeTechnique(Material* m, RenderState* r)
{
	((Dx12RenderState*)r)->CreatePipelineState(((Dx12Material*)m)->vertexBlob, ((Dx12Material*)m)->pixelBlob, rootSignature);

	Technique* t = new Technique(m, r);

	return t;
}

void Dx12Renderer::setResourceTransitionBarrier(ID3D12GraphicsCommandList * commandList, ID3D12Resource * resource, D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter)
{
	D3D12_RESOURCE_BARRIER barrierDesc = {};

	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Transition.pResource = resource;
	barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrierDesc.Transition.StateBefore = StateBefore;
	barrierDesc.Transition.StateAfter = StateAfter;

	commandList->ResourceBarrier(1, &barrierDesc);
}

int Dx12Renderer::initialize(unsigned int width, unsigned int height)
{
	// create the window
	HMODULE hInstance = GetModuleHandle(NULL);
	if (!initializeWindow(hInstance, width, height, false))
	{
		printf("Window initialization failed!");
		return 0;
	}

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

#ifdef _DEBUG
	//Enable the D3D12 debug layer.
	ID3D12Debug* debugController = nullptr;
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	#ifdef STATIC_LINK_DEBUGSTUFF
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
		SafeRelease(debugController);
	#else
		HMODULE mD3D12 = LoadLibrary(L"D3D12.dll");
		PFN_D3D12_GET_DEBUG_INTERFACE f = (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress(mD3D12, "D3D12GetDebugInterface");
		if (SUCCEEDED(f(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
		SafeRelease(&debugController);
	#endif
#else
		UINT compileFlags = 0;
#endif

	// Create Device
	if (adapter) {
		D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device));

		SafeRelease(&adapter);
	}
	else {	// If we did not find a compatible device we create a "warp device"
		factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
		D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));

		SafeRelease(&adapter);
	}
	device->SetName(L"Device");

	// Create fence
	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	fence->SetName(L"Fence");
	fenceValue = 1;
	fenceEvent = CreateEvent(0, false, false, 0);

	SafeRelease(&factory);
	factory = nullptr;
	CreateDXGIFactory(IID_PPV_ARGS(&factory));

	// Create command queues
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
	device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	commandQueue->SetName(L"commandQueue");

	D3D12_COMMAND_QUEUE_DESC computeQueueDesc = { D3D12_COMMAND_LIST_TYPE_COMPUTE, 0, D3D12_COMMAND_QUEUE_FLAG_NONE };
	device->CreateCommandQueue(&computeQueueDesc, IID_PPV_ARGS(&computeCommandQueue));
	commandQueue->SetName(L"computeCommandQueue");

	// Create command allocators and lists
	for (int i = 0; i < frameBufferCount * numThreads; ++i)
	{
		wchar_t name[256];

		device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[i]));
		swprintf_s(name, L"commandAllocator%d", i);
		commandAllocator[i]->SetName(name);

		device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator[i], nullptr, IID_PPV_ARGS(&commandList[i]));
		commandList[i]->Close();
		swprintf_s(name, L"commandList%d", i);
		commandList[i]->SetName(name);

		device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&computeCommandAllocator[i]));
		swprintf_s(name, L"ComputeCommandAllocator%d", i);
		computeCommandAllocator[i]->SetName(name);

		device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, computeCommandAllocator[i], nullptr, IID_PPV_ARGS(&computeCommandList[i]));
		computeCommandList[i]->Close();
		swprintf_s(name, L"ComputeCommandList%d", i);
		computeCommandList[i]->SetName(name);
	}

	// Create swapchain
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = 0;
	swapChainDesc.Height = 0;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = frameBufferCount;
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Flags = 0;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	
	factory->CreateSwapChainForHwnd(
		commandQueue,
		hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		reinterpret_cast<IDXGISwapChain1**>(&swapChain)
	);

	frameIndex = swapChain->GetCurrentBackBufferIndex();
	oldestFrameIndex = frameIndex;

	// Create render targets
	D3D12_DESCRIPTOR_HEAP_DESC dhd = {};
	dhd.NumDescriptors = frameBufferCount;
	dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	device->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&rtvDescriptorHeap));
	rtvDescriptorHeap->SetName(L"RTVDescriptorHeap");

	rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < frameBufferCount; ++i) {
		swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
		device->CreateRenderTargetView(renderTargets[i], nullptr, cdh);
		cdh.ptr += rtvDescriptorSize;
		renderTargets[i]->SetName(L"RenderTarget");
	}

	// Create depth stencil
#pragma region DEPTH STENCIL
	D3D12_DEPTH_STENCILOP_DESC dsopDesc;
	dsopDesc.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	dsopDesc.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	dsopDesc.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	dsopDesc.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	D3D12_DEPTH_STENCIL_DESC dsDesc;
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	dsDesc.StencilEnable = false;
	dsDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	dsDesc.FrontFace = dsopDesc;
	dsDesc.BackFace = dsopDesc;

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsDescriptorHeap));

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&depthStencilBuffer)
	);
	dsDescriptorHeap->SetName(L"Depth/Stencil Resource Heap");

	device->CreateDepthStencilView(depthStencilBuffer, &depthStencilDesc, dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	depthStencilBuffer->SetName(L"DepthStencilBuffer");
#pragma endregion

	// Initialize the viewport
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// Initialize the scissor rect
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = width;
	scissorRect.bottom = height;

	// Define descriptor(table) ranges
	D3D12_DESCRIPTOR_RANGE textureDescriptorTableRange;
	textureDescriptorTableRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	textureDescriptorTableRange.NumDescriptors = 1;
	textureDescriptorTableRange.BaseShaderRegister = 0;
	textureDescriptorTableRange.RegisterSpace = SPACE_PIXEL;
	textureDescriptorTableRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE samplerDescriptorTableRange;
	samplerDescriptorTableRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
	samplerDescriptorTableRange.NumDescriptors = 1;
	samplerDescriptorTableRange.BaseShaderRegister = 0;
	samplerDescriptorTableRange.RegisterSpace = SPACE_PIXEL;
	samplerDescriptorTableRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	
	// Create root parameters
	D3D12_ROOT_PARAMETER rootParam[RS_PARAM_COUNT];
	rootParam[RS_TEXTURES].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[RS_TEXTURES].DescriptorTable.NumDescriptorRanges = 1;
	rootParam[RS_TEXTURES].DescriptorTable.pDescriptorRanges = &textureDescriptorTableRange;
	rootParam[RS_TEXTURES].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	rootParam[RS_SAMPLERS].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[RS_SAMPLERS].DescriptorTable.NumDescriptorRanges = 1;
	rootParam[RS_SAMPLERS].DescriptorTable.pDescriptorRanges = &samplerDescriptorTableRange;
	rootParam[RS_SAMPLERS].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	rootParam[RS_CB_WVP].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParam[RS_CB_WVP].Descriptor.RegisterSpace = SPACE_VERTEX;
	rootParam[RS_CB_WVP].Descriptor.ShaderRegister = 0;
	rootParam[RS_CB_WVP].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	rootParam[RS_CB_COLOR].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParam[RS_CB_COLOR].Descriptor.RegisterSpace = SPACE_PIXEL;
	rootParam[RS_CB_COLOR].Descriptor.ShaderRegister = 0;
	rootParam[RS_CB_COLOR].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	rootParam[RS_SRV_KEYFRAME_CURRENT_POS].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParam[RS_SRV_KEYFRAME_CURRENT_POS].Descriptor.RegisterSpace = SPACE_COMPUTE;
	rootParam[RS_SRV_KEYFRAME_CURRENT_POS].Descriptor.ShaderRegister = 0;
	rootParam[RS_SRV_KEYFRAME_CURRENT_POS].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParam[RS_SRV_KEYFRAME_CURRENT_NOR].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParam[RS_SRV_KEYFRAME_CURRENT_NOR].Descriptor.RegisterSpace = SPACE_COMPUTE;
	rootParam[RS_SRV_KEYFRAME_CURRENT_NOR].Descriptor.ShaderRegister = 1;
	rootParam[RS_SRV_KEYFRAME_CURRENT_NOR].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParam[RS_SRV_KEYFRAME_NEXT_POS].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParam[RS_SRV_KEYFRAME_NEXT_POS].Descriptor.RegisterSpace = SPACE_COMPUTE;
	rootParam[RS_SRV_KEYFRAME_NEXT_POS].Descriptor.ShaderRegister = 2;
	rootParam[RS_SRV_KEYFRAME_NEXT_POS].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParam[RS_SRV_KEYFRAME_NEXT_NOR].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParam[RS_SRV_KEYFRAME_NEXT_NOR].Descriptor.RegisterSpace = SPACE_COMPUTE;
	rootParam[RS_SRV_KEYFRAME_NEXT_NOR].Descriptor.ShaderRegister = 3;
	rootParam[RS_SRV_KEYFRAME_NEXT_NOR].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParam[RS_UAV_MESH_RESULT].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
	rootParam[RS_UAV_MESH_RESULT].Descriptor.RegisterSpace = SPACE_COMPUTE;
	rootParam[RS_UAV_MESH_RESULT].Descriptor.ShaderRegister = 0;
	rootParam[RS_UAV_MESH_RESULT].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParam[RS_CONSTANT_T].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParam[RS_CONSTANT_T].Constants.Num32BitValues = 1;
	rootParam[RS_CONSTANT_T].Constants.RegisterSpace = SPACE_COMPUTE;
	rootParam[RS_CONSTANT_T].Constants.ShaderRegister = 0;
	rootParam[RS_CONSTANT_T].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// Create root signatures
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.NumParameters = RS_PARAM_COUNT;
	rootSignatureDesc.pParameters = rootParam;
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pStaticSamplers = nullptr;
	
	//D3D12_ROOT_SIGNATURE_DESC computeRootSignatureDesc;
	//rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	//rootSignatureDesc.NumParameters = RS_PARAM_COUNT;
	//rootSignatureDesc.pParameters = rootParam;
	//rootSignatureDesc.NumStaticSamplers = 0;
	//rootSignatureDesc.pStaticSamplers = nullptr;

	CD3DX12_ROOT_SIGNATURE_DESC computeRootSignatureDesc(0, NULL, 0, NULL, D3D12_ROOT_SIGNATURE_FLAG_NONE);

	ID3DBlob* sBlob, *errorBlob;
	if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sBlob, &errorBlob)))
	{
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		errorBlob->Release();
	}
	device->CreateRootSignature(0, sBlob->GetBufferPointer(), sBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	rootSignature->SetName(L"rootSignature");

	if (FAILED(D3D12SerializeRootSignature(&computeRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sBlob, &errorBlob)))
	{
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		errorBlob->Release();
	}
	device->CreateRootSignature(0, sBlob->GetBufferPointer(), sBlob->GetBufferSize(), IID_PPV_ARGS(&computeRootSignature));
	rootSignature->SetName(L"computeRootSignature");

	ID3DBlob* computeBlob;
	D3DCompileFromFile(
		L"ComputeShader.hlsl",
		nullptr,
		nullptr,
		"CS_main",
		"cs_5_1",
		compileFlags,
		0,
		&computeBlob,
		nullptr
	);

	D3D12_COMPUTE_PIPELINE_STATE_DESC computePSODesc = {};
	computePSODesc.pRootSignature = computeRootSignature;
	computePSODesc.CS.pShaderBytecode = reinterpret_cast<void*>(computeBlob->GetBufferPointer());
	computePSODesc.CS.BytecodeLength = computeBlob->GetBufferSize();

	device->CreateComputePipelineState(&computePSODesc, IID_PPV_ARGS(&computeState));

	// increment the fence value now, otherwise the buffer might not be uploaded by the time we start drawing
	fenceValue++;
	commandQueue->Signal(fence, fenceValue);

	SafeRelease(&factory);
	return true;
}

void Dx12Renderer::setWinTitle(const char * title)
{
	SetWindowTextA(hwnd, title);
}

void Dx12Renderer::present()
{
	// Present the frame.
	DXGI_PRESENT_PARAMETERS pp = {};
	swapChain->Present1(0, 0, &pp);

	//WaitForGpu(); //Wait for GPU to finish.
				  //NOT BEST PRACTICE, only used as such for simplicity.
}

int Dx12Renderer::shutdown()
{
	// Signal when the fence has increased in value
	const UINT64 signalValue = fenceValue;
	computeCommandQueue->Signal(fence, signalValue);
	fenceValue++; // Increment the comparison value inbefore the next call

	// Wait until the value has been incremented
	if (fence->GetCompletedValue() < signalValue) {
		fence->SetEventOnCompletion(signalValue, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	SafeRelease(&rootSignature);
	SafeRelease(&device);
	SafeRelease(&rtvDescriptorHeap);
	SafeRelease(&dsDescriptorHeap);
	SafeRelease(&depthStencilBuffer);
	SafeRelease(&commandQueue);
	SafeRelease(&swapChain);
	SafeRelease(&fence);

	for (int i = 0; i < frameBufferCount; i++)
		SafeRelease(&renderTargets[i]);

	for (int i = 0; i < frameBufferCount * numThreads; i++) {
		SafeRelease(&commandAllocator[i]);
		SafeRelease(&commandList[i]);
	}

	return 1;
}

void Dx12Renderer::setClearColor(float r, float g, float b, float a)
{
	clearColor[0] = r;
	clearColor[1] = g;
	clearColor[2] = b;
	clearColor[3] = a;
}

void Dx12Renderer::clearBuffer(unsigned int flags)
{
	clearFlags = flags;
}

void Dx12Renderer::submit(Mesh * mesh)
{
	drawList[mesh->technique].push_back(mesh);
}

void Dx12Renderer::frame()
{
	frameIndex = swapChain->GetCurrentBackBufferIndex();

	// Compute
	computeCommandAllocator[frameIndex]->Reset();
	computeCommandList[frameIndex]->Reset(computeCommandAllocator[frameIndex], computeState);
	computeCommandList[frameIndex]->SetPipelineState(computeState);
	computeCommandList[frameIndex]->SetComputeRootSignature(computeRootSignature);

	// Graphics
	commandAllocator[frameIndex]->Reset();
	commandList[frameIndex]->Reset(commandAllocator[frameIndex], NULL);

	setResourceTransitionBarrier(commandList[frameIndex],
		renderTargets[frameIndex],
		D3D12_RESOURCE_STATE_PRESENT,		// state before
		D3D12_RESOURCE_STATE_RENDER_TARGET	// state after
	);

	// Get the handle for the render target we're drawing to (the current back buffer)
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	renderTargetViewHandle.ptr += rtvDescriptorSize * frameIndex;

	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilViewHandle = dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	// Record commands.
	commandList[frameIndex]->OMSetRenderTargets(1, &renderTargetViewHandle, false, &depthStencilViewHandle);

	if (clearFlags & CLEAR_BUFFER_FLAGS::COLOR)
	{
		commandList[frameIndex]->ClearRenderTargetView(renderTargetViewHandle, clearColor, 0, nullptr);
	}
	if (clearFlags & CLEAR_BUFFER_FLAGS::DEPTH)
	{
		commandList[frameIndex]->ClearDepthStencilView(depthStencilViewHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

	commandList[frameIndex]->SetGraphicsRootSignature(rootSignature);
	commandList[frameIndex]->RSSetViewports(1, &viewport);
	commandList[frameIndex]->RSSetScissorRects(1, &scissorRect);
	commandList[frameIndex]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	Dx12Material* material = nullptr;
	Dx12RenderState* renderState = nullptr;
	Dx12Texture2D* texture = nullptr;
	Dx12Sampler2D* sampler = nullptr;
	Dx12VertexBuffer* vBuffer = nullptr;
	Dx12VertexBuffer* nBuffer = nullptr;
	Dx12VertexBuffer* uBuffer = nullptr;
	Dx12IndexBuffer* iBuffer = nullptr;
	Dx12ConstantBuffer* cBuffer = nullptr;

	for (auto work : drawList)
	{
		material = (Dx12Material*)(work.first->getMaterial());
		renderState = (Dx12RenderState*)(work.first->getRenderState());

		commandList[frameIndex]->SetGraphicsRootConstantBufferView(RS_CB_COLOR, material->constantBuffers[DIFFUSE_TINT]->getUploadHeap()->GetGPUVirtualAddress());
		commandList[frameIndex]->SetPipelineState(renderState->getPSO());
		for (auto mesh : work.second)
		{
			size_t numberElements = mesh->geometryBuffers[0].numElements;
			size_t numberIndices = mesh->geometryBuffers[INDEXBUFF].numElements;
			for (auto t : mesh->textures)
			{
				texture = (Dx12Texture2D*)(t.second);
				sampler = (Dx12Sampler2D*)(texture->sampler);

				ID3D12DescriptorHeap* descriptorHeaps[] = { texture->getDescriptorHeap(), sampler->getDescriptorHeap() };
				commandList[frameIndex]->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);

				commandList[frameIndex]->SetGraphicsRootDescriptorTable(RS_TEXTURES, texture->getDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
				commandList[frameIndex]->SetGraphicsRootDescriptorTable(RS_SAMPLERS, sampler->getDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
			}
			vBuffer = (Dx12VertexBuffer*)(mesh->geometryBuffers[POSITION + (mesh->getCurrentKeyframe() * 2)].buffer);
			nBuffer = (Dx12VertexBuffer*)(mesh->geometryBuffers[NORMAL + (mesh->getCurrentKeyframe() * 2)].buffer);
			uBuffer = (Dx12VertexBuffer*)(mesh->geometryBuffers[TEXCOORD].buffer);
			iBuffer = (Dx12IndexBuffer*)(mesh->geometryBuffers[INDEXBUFF].buffer);
			D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[] = { *vBuffer->getView(), *nBuffer->getView(), *uBuffer->getView() };
			commandList[frameIndex]->IASetVertexBuffers(0, ARRAYSIZE(vertexBufferViews), vertexBufferViews);
			commandList[frameIndex]->IASetIndexBuffer(iBuffer->getView());

			mesh->incKeyframe();

			cBuffer = (Dx12ConstantBuffer*)(mesh->wvpBuffer);
			commandList[frameIndex]->SetGraphicsRootConstantBufferView(RS_CB_WVP, cBuffer->getUploadHeap()->GetGPUVirtualAddress());
			vBuffer = (Dx12VertexBuffer*)(((Dx12Mesh*)mesh)->getPosDataCurrent());
			commandList[frameIndex]->SetGraphicsRootShaderResourceView(RS_SRV_KEYFRAME_CURRENT_POS, vBuffer->getUploadHeap()->GetGPUVirtualAddress());
			vBuffer = (Dx12VertexBuffer*)(((Dx12Mesh*)mesh)->getNorDataCurrent());
			commandList[frameIndex]->SetGraphicsRootShaderResourceView(RS_SRV_KEYFRAME_CURRENT_NOR, vBuffer->getUploadHeap()->GetGPUVirtualAddress());
			vBuffer = (Dx12VertexBuffer*)(((Dx12Mesh*)mesh)->getPosDataNext());
			commandList[frameIndex]->SetGraphicsRootShaderResourceView(RS_SRV_KEYFRAME_NEXT_POS, vBuffer->getUploadHeap()->GetGPUVirtualAddress());
			vBuffer = (Dx12VertexBuffer*)(((Dx12Mesh*)mesh)->getPosDataNext());
			commandList[frameIndex]->SetGraphicsRootShaderResourceView(RS_SRV_KEYFRAME_NEXT_NOR, vBuffer->getUploadHeap()->GetGPUVirtualAddress());

			float t = ((Dx12Mesh*)mesh)->getKeyFrameT();
			commandList[frameIndex]->SetGraphicsRoot32BitConstants(RS_CONSTANT_T, 1, &t, 0);

			commandList[frameIndex]->DrawIndexedInstanced(numberIndices, 1, 0, 0, 0);

			// Unbind textures
			for (auto t : mesh->textures)
			{
				texture = (Dx12Texture2D*)(t.second);
				
				ID3D12DescriptorHeap* descriptorHeaps[] = { texture->getNullDescriptorHeap() };
				commandList[frameIndex]->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);

				commandList[frameIndex]->SetGraphicsRootDescriptorTable(RS_TEXTURES, texture->getNullDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
			}
		}
	}

	for (auto work : drawList)
	{
		work.second.clear();
	}
	drawList.clear();


	computeCommandList[frameIndex]->Dispatch(1, 1, 1);
	computeCommandList[frameIndex]->Close();
	ID3D12CommandList* computeListsToExecute[] = { computeCommandList[frameIndex] };
	computeCommandQueue->ExecuteCommandLists(ARRAYSIZE(computeListsToExecute), computeListsToExecute);

	// Execute the command list.
	setResourceTransitionBarrier(commandList[frameIndex],
		renderTargets[frameIndex],
		D3D12_RESOURCE_STATE_RENDER_TARGET,	// state before
		D3D12_RESOURCE_STATE_PRESENT		// state after
	);
	commandList[frameIndex]->Close();
	ID3D12CommandList* commandListsToExecute[] = { commandList[frameIndex] };
	commandQueue->ExecuteCommandLists(ARRAYSIZE(commandListsToExecute), commandListsToExecute);

	fenceValues[frameIndex] = fenceValue;
	commandQueue->Signal(fence, fenceValue++);

	if (fence->GetCompletedValue() < fenceValues[oldestFrameIndex]) {
		fence->SetEventOnCompletion(fenceValues[oldestFrameIndex], fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
		if (++oldestFrameIndex >= frameBufferCount)
			oldestFrameIndex = 0;
	}
}

bool Dx12Renderer::initializeWindow(HINSTANCE hInstance, int width, int height, bool fullscreen)
{
	if (fullscreen)
	{
		HMONITOR hmon = MonitorFromWindow(hwnd,
			MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(hmon, &mi);

		width = mi.rcMonitor.right - mi.rcMonitor.left;
		height = mi.rcMonitor.bottom - mi.rcMonitor.top;
	}

	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WindowName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		printf("Error creating window registers!");
		return false;
	}

	hwnd = CreateWindowEx(NULL,
		WindowName,
		WindowTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (!hwnd)
	{
		printf("Error creating window!");
		return false;
	}

	if (fullscreen)
	{
		SetWindowLong(hwnd, GWL_STYLE, 0);
	}

	ShowWindow(hwnd, SW_SHOW);
	//ShowWindow(GetConsoleWindow(), SW_HIDE);
	UpdateWindow(hwnd);

	RECT rcClip;
	GetWindowRect(hwnd, &rcClip);
	ClipCursor(&rcClip);

	POINT pos;
	pos.x = width / 2;
	pos.y = height / 2;
	ClientToScreen(hwnd, &pos);
	SetCursorPos(pos.x, pos.y);
	ShowCursor(false);
	
	return true;
}

void Dx12Renderer::WaitForGpu()
{
	// Signal when the fence has increased in value
	const UINT64 signalValue = fenceValue;
	commandQueue->Signal(fence, signalValue);

	// Wait until the value has been incremented
	if (fence->GetCompletedValue() < signalValue) {
		fence->SetEventOnCompletion(signalValue, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	fenceValue++; // Increment the comparison value inbefore the next call
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			if (MessageBox(0, L"Are you sure you want to exit?",
				L"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
				DestroyWindow(hWnd);
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd,
		msg,
		wParam,
		lParam);
}
