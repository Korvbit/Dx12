#include "Dx12/Dx12Renderer.h"
#include "Renderer.h"


Renderer* Renderer::makeRenderer(BACKEND option)
{
	return new Dx12Renderer();
}

Renderer::~Renderer()
{
}
