#include "Dx12Camera.h"
#include <stdio.h>

Dx12Camera::Dx12Camera(int screenWidth, int screenHeight)
{
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	fov = 45.0f*(3.14f / 180.0f);
	aspect = (float)m_screenWidth / (float)m_screenHeight;
	nearPlane = 0.1;
	farPlane = 1000.0f;

	position = { 0.0f, 0.0f, -4.0f, 0.0f };
	forward = { 0.0f, 0.0f, 1.0f, 0.0f };
	right = { 1.0f, 0.0f, 0.0f, 0.0f };
	up = { 0.0f, 1.0f, 0.0f, 0.0f };

	stepF = 0;
	stepL = 0;
	stepB = 0;
	stepR = 0;
	stepU = 0;
	stepD = 0;
	rotR = 0;
	rotL = 0;
	rotU = 0;
	rotD = 0;
	speed = FAST_SPEED;
}

Dx12Camera::~Dx12Camera()
{
}

void Dx12Camera::startMove(WPARAM key)
{
	switch (key)
	{
	case KEY_A:
		stepL = 1;
		break;

	case KEY_D:
		stepR = 1;
		break;

	case KEY_W:
		stepF = 1;
		break;

	case KEY_S:
		stepB = 1;
		break;

	case KEY_E:
		stepU = 1;
		break;

	case KEY_Q:
		stepD = 1;
		break;

	case VK_LEFT:
		rotL = 1;
		break;

	case VK_RIGHT:
		rotR = 1;
		break;

	case VK_UP:
		rotU = 1;
		break;

	case VK_DOWN:
		rotD = 1;
		break;

	case VK_SHIFT:
		speed = SLOW_SPEED;
		break;

	default:
		break;
	}
}

void Dx12Camera::endMove(WPARAM key)
{
	switch (key)
	{
	case KEY_A:
		stepL = 0;
		break;

	case KEY_D:
		stepR = 0;
		break;

	case KEY_W:
		stepF = 0;
		break;

	case KEY_S:
		stepB = 0;
		break;

	case KEY_E:
		stepU = 0;
		break;

	case KEY_Q:
		stepD = 0;
		break;

	case VK_LEFT:
		rotL = 0;
		break;

	case VK_RIGHT:
		rotR = 0;
		break;

	case VK_UP:
		rotU = 0;
		break;

	case VK_DOWN:
		rotD = 0;
		break;

	case VK_SHIFT:
		speed = FAST_SPEED;
		break;

	case VK_SPACE:
		reset();
		break;

	default:
		break;
	}
}

void Dx12Camera::rotate(float movementX, float movementY)
{
	float angleX = movementX / 1000.0f;
	DirectX::XMVECTOR rotate = DirectX::XMQuaternionRotationRollPitchYaw(0.0f, angleX, 0.0f);
	DirectX::XMStoreFloat4(&forward, DirectX::XMVector3Rotate(DirectX::XMLoadFloat4(&forward), rotate));
	DirectX::XMStoreFloat4(&right, DirectX::XMVector3Rotate(DirectX::XMLoadFloat4(&right), rotate));
	DirectX::XMStoreFloat4(&up, DirectX::XMVector3Rotate(DirectX::XMLoadFloat4(&up), rotate));
	
	float angleY = movementY / 1000.0f;
	rotate = DirectX::XMQuaternionRotationAxis(DirectX::XMLoadFloat4(&right), angleY);
	DirectX::XMStoreFloat4(&forward, DirectX::XMVector3Rotate(DirectX::XMLoadFloat4(&forward), rotate));
	DirectX::XMStoreFloat4(&up, DirectX::XMVector3Rotate(DirectX::XMLoadFloat4(&up), rotate));
}

void Dx12Camera::reset()
{
	fov = 45.0f*(3.14f / 180.0f);
	aspect = (float)m_screenWidth / (float)m_screenHeight;
	nearPlane = 0.1;
	farPlane = 1000.0f;

	position = { 0.0f, 0.0f, -4.0f, 0.0f };
	forward = { 0.0f, 0.0f, 1.0f, 0.0f };
	right = { 1.0f, 0.0f, 0.0f, 0.0f };
	up = { 0.0f, 1.0f, 0.0f, 0.0f };

	stepF = 0;
	stepL = 0;
	stepB = 0;
	stepR = 0;
	stepU = 0;
	stepD = 0;
	rotR = 0;
	rotL = 0;
	rotU = 0;
	rotD = 0;
	speed = 10;
}

void Dx12Camera::Update(float dt)
{
	position.x += forward.x * speed * dt * (stepF - stepB);
	position.y += forward.y * speed * dt * (stepF - stepB);
	position.z += forward.z * speed * dt * (stepF - stepB);

	position.x += right.x * speed * dt * (stepR - stepL);
	position.y += right.y * speed * dt * (stepR - stepL);
	position.z += right.z * speed * dt * (stepR - stepL);

	position.x += up.x * speed * dt * (stepU - stepD);
	position.y += up.y * speed * dt * (stepU - stepD);
	position.z += up.z * speed * dt * (stepU - stepD);

	rotate(pow(FAST_SPEED, 4) * dt * (rotR - rotL), pow(FAST_SPEED, 4) * dt * (rotD - rotU));

	// Create projection matrix
	DirectX::XMMATRIX tmpMat = DirectX::XMMatrixPerspectiveFovLH(fov, aspect, nearPlane, farPlane);

	// Create view matrix, multiply the matrices and store vpMatrix
	DirectX::XMVECTOR cPos = DirectX::XMLoadFloat4(&position);
	DirectX::XMVECTOR cFor = DirectX::XMLoadFloat4(&forward);
	DirectX::XMVECTOR cRig = DirectX::XMLoadFloat4(&right);
	DirectX::XMVECTOR cUp = DirectX::XMLoadFloat4(&up);
	DirectX::XMVECTOR cTarg = DirectX::XMVectorAdd(cFor, cPos);
	tmpMat = DirectX::XMMatrixLookAtLH(cPos, cTarg, cUp) * tmpMat;
	XMStoreFloat4x4(&vpMatrix, tmpMat);
}
