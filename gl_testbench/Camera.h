#pragma once
#include <DirectXMath.h>

class Camera
{
public:
	Camera();
	~Camera();

	static Camera* makeCamera(int screenWidth, int screenHeight);
	virtual void Update() = 0;

	int m_screenWidth;
	int m_screenHeight;
};

