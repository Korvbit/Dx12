#include "Dx12Renderer.h"

Dx12Renderer::Dx12Renderer()
{
}

Dx12Renderer::~Dx12Renderer()
{
	//std::unordered_map<Technique*, std::vector<Mesh*>> drawList;

	rootSignature->Release();

	device->Release();
	rtvDescriptorHeap->Release();
	dsDescriptorHeap->Release();
	depthStencilBuffer->Release();
	commandQueue->Release();
	commandList->Release();
	swapChain->Release();

	for (int i = 0; i < frameBufferCount; i++) {
		renderTargets[i]->Release();
		commandAllocator[i]->Release();
		fence[i]->Release();
	}
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
	return (Texture2D*)new Dx12Texture2D(device, commandList, commandQueue, commandAllocator[frameIndex]);
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

ConstantBuffer * Dx12Renderer::makeConstantBuffer(std::string NAME, unsigned int location)
{
	return new Dx12ConstantBuffer(NAME, location, device);
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
	for (int i = 0; i < frameBufferCount; ++i)
		device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[i]));

	// Create command lists
	device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandAllocator[0],
		nullptr,
		IID_PPV_ARGS(&commandList));


	// Create fences
	for (int i = 0; i < frameBufferCount; ++i)
	{
		device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence[i]));
		fenceValue[i] = 1;
		fenceEvent = CreateEvent(0, false, false, 0);
	}

	SafeRelease(&factory);
	factory = nullptr;
	CreateDXGIFactory(IID_PPV_ARGS(&factory));

	// Create swapchain
	IDXGISwapChain1* swapChain1 = nullptr;
	
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.Width = 0;
	scDesc.Height = 0;
	scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.Stereo = FALSE;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = frameBufferCount;
	scDesc.Scaling = DXGI_SCALING_NONE;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.Flags = 0;
	scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	
	factory->CreateSwapChainForHwnd(
		commandQueue,
		hwnd,
		&scDesc,
		nullptr,
		nullptr,
		reinterpret_cast<IDXGISwapChain1**>(&swapChain)
	);

	frameIndex = swapChain->GetCurrentBackBufferIndex();


	// Create render target descriptors
	D3D12_DESCRIPTOR_HEAP_DESC dhd = {};
	dhd.NumDescriptors = frameBufferCount;
	dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	device->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&rtvDescriptorHeap));

	rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	// Create render targets
	for (UINT i = 0; i < frameBufferCount; ++i) {
		swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
		device->CreateRenderTargetView(renderTargets[i], nullptr, cdh);
		cdh.ptr += rtvDescriptorSize;
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
	D3D12_DESCRIPTOR_RANGE dtRanges[1];
	dtRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	dtRanges[0].NumDescriptors = 1;
	dtRanges[0].BaseShaderRegister = 0;
	dtRanges[0].RegisterSpace = 0;
	dtRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE samplerRanges[1];
	samplerRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
	samplerRanges[0].NumDescriptors = 1;
	samplerRanges[0].BaseShaderRegister = 0;
	samplerRanges[0].RegisterSpace = 0;
	samplerRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// Create descriptor table
	D3D12_ROOT_DESCRIPTOR_TABLE dt;
	dt.NumDescriptorRanges = ARRAYSIZE(dtRanges);
	dt.pDescriptorRanges = dtRanges;

	D3D12_ROOT_DESCRIPTOR_TABLE dtSampler;
	dtSampler.NumDescriptorRanges = ARRAYSIZE(samplerRanges);
	dtSampler.pDescriptorRanges = samplerRanges;

	// Create Constant Buffer root descriptor
	D3D12_ROOT_DESCRIPTOR rootCBVDescriptor;
	rootCBVDescriptor.RegisterSpace = 0;
	rootCBVDescriptor.ShaderRegister = 0;
	
	// Create root parameter
	D3D12_ROOT_PARAMETER rootParam[4];
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].DescriptorTable = dt;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[1].DescriptorTable = dtSampler;
	rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	rootParam[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParam[2].Descriptor = rootCBVDescriptor;
	rootParam[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	rootCBVDescriptor.ShaderRegister = 1;

	rootParam[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParam[3].Descriptor = rootCBVDescriptor;
	rootParam[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// Create root signature
	D3D12_ROOT_SIGNATURE_DESC rsDesc;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rsDesc.NumParameters = ARRAYSIZE(rootParam);
	rsDesc.pParameters = rootParam;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.pStaticSamplers = nullptr;

	ID3DBlob* sBlob;
	D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sBlob, nullptr);

	device->CreateRootSignature(0, sBlob->GetBufferPointer(), sBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

	// Create constant buffer ================================================

	commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// increment the fence value now, otherwise the buffer might not be uploaded by the time we start drawing
	fenceValue[frameIndex]++;
	commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);

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

	WaitForGpu(); //Wait for GPU to finish.
				  //NOT BEST PRACTICE, only used as such for simplicity.
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
	frameIndex = swapChain->GetCurrentBackBufferIndex();

	commandAllocator[frameIndex]->Reset();
	commandList->Reset(commandAllocator[frameIndex], NULL);
	
	setResourceTransitionBarrier(commandList,
		renderTargets[frameIndex],
		D3D12_RESOURCE_STATE_PRESENT,		// state before
		D3D12_RESOURCE_STATE_RENDER_TARGET	// state after
	);

	// Get the handle for the render target we're drawing to (the current back buffer)
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	cdh.ptr += rtvDescriptorSize * frameIndex;

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	// Record commands.
	commandList->OMSetRenderTargets(1, &cdh, false, &dsvHandle);

	if (flags & CLEAR_BUFFER_FLAGS::COLOR)
	{
		commandList->ClearRenderTargetView(cdh, clearColor, 0, nullptr);
	}
	if (flags & CLEAR_BUFFER_FLAGS::DEPTH)
	{
		commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}
}

void Dx12Renderer::submit(Mesh * mesh)
{
	drawList[mesh->technique].push_back(mesh);
}

void Dx12Renderer::frame()
{
	commandList->SetGraphicsRootSignature(rootSignature);
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	Dx12Material* material;
	Dx12RenderState* renderState;
	Dx12Texture2D* texture;
	Dx12Sampler2D* sampler;
	Dx12VertexBuffer* vBuffer;
	Dx12VertexBuffer* nBuffer;
	Dx12VertexBuffer* uBuffer;
	Dx12ConstantBuffer* cBuffer;

	for (auto work : drawList)
	{
		material = (Dx12Material*)(work.first->getMaterial());
		renderState = (Dx12RenderState*)(work.first->getRenderState());

		commandList->SetGraphicsRootConstantBufferView(3, material->constantBuffers[DIFFUSE_TINT]->getUploadHeap()->GetGPUVirtualAddress());
		commandList->SetPipelineState(renderState->getPSO());
		for (auto mesh : work.second)
		{
			size_t numberElements = mesh->geometryBuffers[0].numElements;
			for (auto t : mesh->textures)
			{
				texture = (Dx12Texture2D*)(t.second);
				sampler = (Dx12Sampler2D*)(texture->sampler);

				ID3D12DescriptorHeap* descriptorHeaps[] = { texture->getDescriptorHeap(), sampler->getDescriptorHeap() };
				commandList->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);

				commandList->SetGraphicsRootDescriptorTable(0, texture->getDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
				commandList->SetGraphicsRootDescriptorTable(1, sampler->getDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
			}
			vBuffer = (Dx12VertexBuffer*)(mesh->geometryBuffers[POSITION].buffer);
			nBuffer = (Dx12VertexBuffer*)(mesh->geometryBuffers[NORMAL].buffer);
			uBuffer = (Dx12VertexBuffer*)(mesh->geometryBuffers[TEXCOORD].buffer);
			D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[] = { *vBuffer->getView(), *nBuffer->getView(), *uBuffer->getView() };
			commandList->IASetVertexBuffers(0, ARRAYSIZE(vertexBufferViews), vertexBufferViews);

			cBuffer = (Dx12ConstantBuffer*)(mesh->txBuffer);
			commandList->SetGraphicsRootConstantBufferView(2, cBuffer->getUploadHeap()->GetGPUVirtualAddress());
			commandList->DrawInstanced(6, 1, 0, 0);
		}
	}

	drawList.clear();

	setResourceTransitionBarrier(commandList,
		renderTargets[frameIndex],
		D3D12_RESOURCE_STATE_RENDER_TARGET,	// state before
		D3D12_RESOURCE_STATE_PRESENT		// state after
	);

	// Close the list to prepare it for execution.
	commandList->Close();

	// Execute the command list.
	ID3D12CommandList* listsToExecute[] = { commandList };
	commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);
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
	UpdateWindow(hwnd);

	return true;
}

void Dx12Renderer::WaitForGpu()
{
	// Signal when the fence has increased in value
	const UINT64 signalValue = fenceValue[frameIndex];
	commandQueue->Signal(fence[frameIndex], signalValue);
	fenceValue[frameIndex]++; // Increment the comparison value inbefore the next call

	// Wait until the value has been incremented
	if (fence[frameIndex]->GetCompletedValue() < signalValue) {
		fence[frameIndex]->SetEventOnCompletion(signalValue, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
	}
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
