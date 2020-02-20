#include "Camera.h"



Camera::Camera()
{
}

Camera::~Camera()
{
}

Camera * Camera::makeCamera(int screenWidth, int screenHeight)
{
	return new Camera(screenWidth, screenHeight);
}
