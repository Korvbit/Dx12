#pragma once
#include <DirectXMath.h>

class Camera
{
public:
	Camera();
	~Camera();

	virtual void Update() = 0;

	int m_screenWidth;
	int m_screenHeight;
};

