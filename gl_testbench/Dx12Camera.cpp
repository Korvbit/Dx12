#include "Dx12Camera.h"

Dx12Camera::Dx12Camera(int screenWidth, int screenHeight)
{
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	position = { 0.0f, 0.0f, -4.0f, 0.0f};
	target = { 0.0f, 0.0f, 0.0f, 0.0f };
	up = { 0.0f, 1.0f, 0.0f, 0.0f };
}

Dx12Camera::~Dx12Camera()
{
}

void Dx12Camera::move(float3 direction)
{
	position.x += direction.x;
	position.y += direction.y;
	position.z += direction.z;
	target.x += direction.x;
	target.y += direction.y;
	target.z += direction.z;
}

void Dx12Camera::Update()
{
	// Create projection matrix
	DirectX::XMMATRIX tmpMat = DirectX::XMMatrixPerspectiveFovLH(45.0f*(3.14f / 180.0f), (float)m_screenWidth / (float)m_screenHeight, 0.1f, 1000.0f);

	// Create view matrix, multiply the matrices and store vpMatrix
	DirectX::XMVECTOR cPos = DirectX::XMLoadFloat4(&position);
	DirectX::XMVECTOR cTarg = DirectX::XMLoadFloat4(&target);
	DirectX::XMVECTOR cUp = DirectX::XMLoadFloat4(&up);
	tmpMat = DirectX::XMMatrixLookAtLH(cPos, cTarg, cUp) * tmpMat;
	XMStoreFloat4x4(&vpMatrix, tmpMat);
}
