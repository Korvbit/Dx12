#include "Dx12Texture2D.h"

Dx12Texture2D::Dx12Texture2D()
{
}

Dx12Texture2D::~Dx12Texture2D()
{
}

int Dx12Texture2D::loadFromFile(std::string filename)
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

	hr = wicFactory->CreateDecoderFromFilename(
		filename,
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

	resourceDescription = {};
	resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDescription.Alignment = 0;
	resourceDescription.Width = textureWidth;
	resourceDescription.Height = textureHeight;
	resourceDescription.DepthOrArraySize = 1;
	resourceDescription.MipLevels = 1;
	resourceDescription.Format = dxgiFormat;
	resourceDescription.SampleDesc.Count = 1;
	resourceDescription.SampleDesc.Quality = 0;
	resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDescription.Flags = D3D12_RESOURCE_FLAG_NONE;

	return imageSize;
}
