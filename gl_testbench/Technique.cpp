#include <iostream>
#include "Technique.h"
#include "Renderer.h"

Technique::~Technique()
{
	std::cout << "destroyed technique" << std::endl;
}


