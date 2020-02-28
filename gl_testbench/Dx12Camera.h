#pragma once
#include "Camera.h"
#include <DirectXMath.h>

class Dx12Camera : public Camera 
{
public:
	Dx12Camera(int screenWidth, int screenHeight);
	~Dx12Camera();

	void move(float3 direction);
	void Update();

	DirectX::XMFLOAT4 position;
	DirectX::XMFLOAT4 target;
	DirectX::XMFLOAT4 up;

	DirectX::XMFLOAT4X4 vpMatrix;
};