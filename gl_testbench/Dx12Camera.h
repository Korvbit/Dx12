#pragma once
#include "Camera.h"
#include <DirectXMath.h>
#include "Dx12/functions.h"

#define FAST_SPEED 6
#define SLOW_SPEED 2

class Dx12Camera : public Camera 
{
public:
	Dx12Camera(int screenWidth, int screenHeight);
	~Dx12Camera();

	void startMove(WPARAM key);
	void endMove(WPARAM key);
	void rotate(float movementX, float movementY);
	void reset();
	void Update(float dt);

	DirectX::XMFLOAT4 position;
	DirectX::XMFLOAT4 forward;
	DirectX::XMFLOAT4 right;
	DirectX::XMFLOAT4 up;
	float fov;
	float aspect;
	float nearPlane;
	float farPlane;

	int stepF;
	int stepL;
	int stepB;
	int stepR;
	int stepU;
	int stepD;
	int rotR;
	int rotL;
	int rotU;
	int rotD;
	int speed;

	DirectX::XMFLOAT4X4 vpMatrix;
};