#include <iostream>
#include "Technique.h"
#include "Renderer.h"

Technique::~Technique()
{
	if (renderState)
	{
		delete renderState;
		renderState = nullptr;
	}
	std::cout << "destroyed technique" << std::endl;
}


