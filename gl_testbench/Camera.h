#pragma once
#include "IA.h"

class Camera
{
public:
	Camera();
	~Camera();

	virtual void move(float3 direction) = 0;
	virtual void Update() = 0;

	int m_screenWidth;
	int m_screenHeight;
};

