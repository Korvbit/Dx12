#include "Dx12Texture2D.h"

Dx12Texture2D::Dx12Texture2D(ID3D12Device* rendererDevice, ID3D12GraphicsCommandList* rendererCommandList, ID3D12CommandQueue* rendererCommandQueue, ID3D12CommandAllocator* rendererCommandAllocator)
{
	device = rendererDevice;
	commandList = rendererCommandList;
	commandQueue = rendererCommandQueue;
	commandAllocator = rendererCommandAllocator;

	D3D12_DESCRIPTOR_HEAP_DESC heapDescriptorDesc = {};
	heapDescriptorDesc.NumDescriptors = 1;
	heapDescriptorDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDescriptorDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	device->CreateDescriptorHeap(&heapDescriptorDesc, IID_PPV_ARGS(&descriptorHeap));

	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	fenceValue = 1;
	fenceEvent = CreateEvent(0, false, false, 0);
}

Dx12Texture2D::~Dx12Texture2D()
{
}

int Dx12Texture2D::loadFromFile(std::string filename)
{
	HRESULT hr;

	commandAllocator->Reset();
	commandList->Reset(commandAllocator, NULL);

	static IWICImagingFactory *wicFactory;

	IWICBitmapDecoder *wicDecoder = NULL;
	IWICBitmapFrameDecode *wicFrame = NULL;
	IWICFormatConverter *wicConverter = NULL;

	bool imageConverted = false;

	if (wicFactory == NULL)
	{
		CoInitialize(NULL);

		hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&wicFactory)
		);
		if (FAILED(hr)) return 0;
	}

	std::wstring stemp = std::wstring(filename.begin(), filename.end());
	LPCWSTR sw = stemp.c_str();

	hr = wicFactory->CreateDecoderFromFilename(
		sw,
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&wicDecoder
	);
	if (FAILED(hr)) return 0;

	// Get the decoded image
	hr = wicDecoder->GetFrame(0, &wicFrame);
	if (FAILED(hr)) return 0;

	// Get wic pixel format of image
	WICPixelFormatGUID pixelFormat;
	hr = wicFrame->GetPixelFormat(&pixelFormat);
	if (FAILED(hr)) return 0;

	// Get size of image
	UINT textureWidth, textureHeight;
	hr = wicFrame->GetSize(&textureWidth, &textureHeight);
	if (FAILED(hr)) return 0;

	// Convert wic pixel format to dxgi pixel format
	DXGI_FORMAT dxgiFormat = GetDXGIFormatFromWICFormat(pixelFormat);

	int bitsPerPixel = GetDXGIFormatBitsPerPixel(dxgiFormat);
	bytesPerRow = (textureWidth * bitsPerPixel) / 8;
	int imageSize = bytesPerRow * textureHeight;

	imageData = (BYTE*)malloc(imageSize);

	// Copy the data from wicFrame to imageData
	hr = wicFrame->CopyPixels(0, bytesPerRow, imageSize, imageData);
	if (FAILED(hr)) return 0;

	resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = textureWidth;
	resourceDesc.Height = textureHeight;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = dxgiFormat;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&resourceDesc, // the description of our texture
		D3D12_RESOURCE_STATE_COPY_DEST, // We will copy the texture from the upload heap to here, so we start it out in a copy dest state
		nullptr, // used for render targets and depth/stencil buffers
		IID_PPV_ARGS(&textureBuffer));

	textureBuffer->SetName(L"Texture Buffer Resource Heap");

	UINT64 textureUploadBufferSize;
	// this function gets the size an upload buffer needs to be to upload a texture to the gpu.
	// each row must be 256 byte aligned except for the last row, which can just be the size in bytes of the row
	// eg. textureUploadBufferSize = ((((width * numBytesPerPixel) + 255) & ~255) * (height - 1)) + (width * numBytesPerPixel);
	//textureUploadBufferSize = (((imageBytesPerRow + 255) & ~255) * (textureDesc.Height - 1)) + imageBytesPerRow;
	device->GetCopyableFootprints(&resourceDesc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);

	// now we create an upload heap to upload our texture to the GPU
	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize), // resource description for a buffer (storing the image data in this heap just to copy to the default heap)
		D3D12_RESOURCE_STATE_GENERIC_READ, // We will copy the contents from this heap to the default heap above
		nullptr,
		IID_PPV_ARGS(&textureBufferUploadHeap));

	textureBufferUploadHeap->SetName(L"Texture Buffer Upload Resource Heap");

	// store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = imageData; // pointer to our image data
	textureData.RowPitch = bytesPerRow; // size of all our triangle vertex data
	textureData.SlicePitch = bytesPerRow * resourceDesc.Height; // also the size of our triangle vertex data

	// Now we copy the upload buffer contents to the default heap
	UpdateSubresources(commandList, textureBuffer, textureBufferUploadHeap, 0, 0, 1, &textureData);

	// transition the texture default heap to a pixel shader resource (we will be sampling from this heap in the pixel shader to get the color of pixels)
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	// now we create a shader resource view (descriptor that points to the texture and describes it)
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = resourceDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(textureBuffer, &srvDesc, descriptorHeap->GetCPUDescriptorHandleForHeapStart());

	commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Signal when the fence has increased in value
	const UINT64 signalValue = fenceValue;
	commandQueue->Signal(fence, signalValue);
	fenceValue++; // Increment the comparison value inbefore the next call

	// Wait until the value has been incremented
	if (fence->GetCompletedValue() < signalValue) {
		fence->SetEventOnCompletion(signalValue, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	return imageSize;
}

int Dx12Texture2D::loadImageFromFile(std::string filename)
{
	HRESULT hr;

	static IWICImagingFactory *wicFactory;

	IWICBitmapDecoder *wicDecoder = NULL;
	IWICBitmapFrameDecode *wicFrame = NULL;
	IWICFormatConverter *wicConverter = NULL;

	bool imageConverted = false;

	if (wicFactory == NULL)
	{
		CoInitialize(NULL);

		hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&wicFactory)
		);
		if (FAILED(hr)) return 0;
	}

	std::wstring stemp = std::wstring(filename.begin(), filename.end());
	LPCWSTR sw = stemp.c_str();

	hr = wicFactory->CreateDecoderFromFilename(
		sw,
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&wicDecoder
	);
	if (FAILED(hr)) return 0;

	// Get the decoded image
	hr = wicDecoder->GetFrame(0, &wicFrame);
	if (FAILED(hr)) return 0;

	// Get wic pixel format of image
	WICPixelFormatGUID pixelFormat;
	hr = wicFrame->GetPixelFormat(&pixelFormat);
	if (FAILED(hr)) return 0;

	// Get size of image
	UINT textureWidth, textureHeight;
	hr = wicFrame->GetSize(&textureWidth, &textureHeight);
	if (FAILED(hr)) return 0;

	// Convert wic pixel format to dxgi pixel format
	DXGI_FORMAT dxgiFormat = GetDXGIFormatFromWICFormat(pixelFormat);

	int bitsPerPixel = GetDXGIFormatBitsPerPixel(dxgiFormat);
	bytesPerRow = (textureWidth * bitsPerPixel) / 8;
	int imageSize = bytesPerRow * textureHeight;

	imageData = (BYTE*)malloc(imageSize);

	// Copy the data from wicFrame to imageData
	hr = wicFrame->CopyPixels(0, bytesPerRow, imageSize, imageData);
	if (FAILED(hr)) return 0;

	resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = textureWidth;
	resourceDesc.Height = textureHeight;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = dxgiFormat;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	return imageSize;
}

ID3D12DescriptorHeap * Dx12Texture2D::getDescriptorHeap()
{
	return descriptorHeap;
}

// get the dxgi format equivilent of a wic format
DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID)
{
	if (wicFormatGUID == GUID_WICPixelFormat128bppRGBAFloat) return DXGI_FORMAT_R32G32B32A32_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBAHalf) return DXGI_FORMAT_R16G16B16A16_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBA) return DXGI_FORMAT_R16G16B16A16_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA) return DXGI_FORMAT_R8G8B8A8_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppBGRA) return DXGI_FORMAT_B8G8R8A8_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppBGR) return DXGI_FORMAT_B8G8R8X8_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA1010102XR) return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;

	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA1010102) return DXGI_FORMAT_R10G10B10A2_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppBGRA5551) return DXGI_FORMAT_B5G5R5A1_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppBGR565) return DXGI_FORMAT_B5G6R5_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppGrayFloat) return DXGI_FORMAT_R32_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppGrayHalf) return DXGI_FORMAT_R16_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppGray) return DXGI_FORMAT_R16_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat8bppGray) return DXGI_FORMAT_R8_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat8bppAlpha) return DXGI_FORMAT_A8_UNORM;

	else return DXGI_FORMAT_UNKNOWN;
}

// get the number of bits per pixel for a dxgi format
int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat)
{
	if (dxgiFormat == DXGI_FORMAT_R32G32B32A32_FLOAT) return 128;
	else if (dxgiFormat == DXGI_FORMAT_R16G16B16A16_FLOAT) return 64;
	else if (dxgiFormat == DXGI_FORMAT_R16G16B16A16_UNORM) return 64;
	else if (dxgiFormat == DXGI_FORMAT_R8G8B8A8_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_B8G8R8A8_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_B8G8R8X8_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM) return 32;

	else if (dxgiFormat == DXGI_FORMAT_R10G10B10A2_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_B5G5R5A1_UNORM) return 16;
	else if (dxgiFormat == DXGI_FORMAT_B5G6R5_UNORM) return 16;
	else if (dxgiFormat == DXGI_FORMAT_R32_FLOAT) return 32;
	else if (dxgiFormat == DXGI_FORMAT_R16_FLOAT) return 16;
	else if (dxgiFormat == DXGI_FORMAT_R16_UNORM) return 16;
	else if (dxgiFormat == DXGI_FORMAT_R8_UNORM) return 8;
	else if (dxgiFormat == DXGI_FORMAT_A8_UNORM) return 8;
}