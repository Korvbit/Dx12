#include "OpenGL/OpenGLRenderer.h"
#include "Dx12/Dx12Renderer.h"
#include "Renderer.h"


Renderer* Renderer::makeRenderer(BACKEND option)
{
	if (option == BACKEND::GL45)
		return new OpenGLRenderer();
	else if (option == BACKEND::DX12)
		return new Dx12Renderer();
}

