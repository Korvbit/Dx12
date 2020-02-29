#pragma once
#include "IA.h"
#include <windows.h>

class Camera
{
public:
	Camera();
	~Camera();

	virtual void startMove(WPARAM key) = 0;
	virtual void endMove(WPARAM key) = 0;
	virtual void rotate(int movementX, int movementY) = 0;
	virtual void reset() = 0;
	virtual void Update(float dt) = 0;

	int m_screenWidth;
	int m_screenHeight;
};

