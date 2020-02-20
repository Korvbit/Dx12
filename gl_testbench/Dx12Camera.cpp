#include "Dx12Camera.h"

Dx12Camera::Dx12Camera(int screenWidth, int screenHeight)
{
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	position = { 0.0f, 2.0f, -4.0f, 0.0f };
	target = { 0.0f, 0.0f, 0.0f, 0.0f };
	up = { 0.0f, 1.0f, 0.0f, 0.0f };

	// Create projection matrix
	DirectX::XMMATRIX tmpMat = DirectX::XMMatrixPerspectiveFovLH(45.0f*(3.14f / 180.0f), (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f);
	DirectX::XMStoreFloat4x4(&projectionMatrix, tmpMat);

	// Create view matrix
	DirectX::XMVECTOR cPos = DirectX::XMLoadFloat4(&position);
	DirectX::XMVECTOR cTarg = DirectX::XMLoadFloat4(&target);
	DirectX::XMVECTOR cUp = DirectX::XMLoadFloat4(&up);
	tmpMat = DirectX::XMMatrixLookAtLH(cPos, cTarg, cUp);
	XMStoreFloat4x4(&viewMatrix, tmpMat);
}

Dx12Camera::~Dx12Camera()
{
}

void Dx12Camera::Update()
{
}
