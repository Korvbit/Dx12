#pragma once
#include "Camera.h"

class Dx12Camera : public Camera 
{
public:
	Dx12Camera(int screenWidth, int screenHeight);
	~Dx12Camera();

	void Update();

	DirectX::XMFLOAT4 position;
	DirectX::XMFLOAT4 target;
	DirectX::XMFLOAT4 up;

	DirectX::XMFLOAT4X4 projectionMatrix;
	DirectX::XMFLOAT4X4 viewMatrix;
};