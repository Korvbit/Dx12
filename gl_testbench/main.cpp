#include <string>
#include <SDL_keyboard.h>
#include <SDL_events.h>
#include <SDL_timer.h>
#include <type_traits> 
#include <assert.h>

#include "Renderer.h"
#include "Mesh.h"
#include "Texture2D.h"
#include <math.h>
#include "Dx12/functions.h"
#include "Camera.h"
#include <windowsx.h>

//#include "vld.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

using namespace std;
Camera* camera;
Renderer* renderer;

// flat scene at the application level...we don't care about this here.
// do what ever you want in your renderer backend.
// all these objects are loosely coupled, creation and destruction is responsibility
// of the testbench, not of the container objects
vector<Mesh*> scene;
vector<Material*> materials;
vector<Technique*> techniques;
vector<Texture2D*> textures;
vector<Sampler2D*> samplers;

// forward decls
void updateScene();
void renderScene();

char gTitleBuff[256];
double gLastDelta = 0.0;
float dt = 0.0f;
float fpsCounter = 0.0f;
int frameCounter = 0;
int fps = 0;
bool shiftStep = true;
POINT center = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };

void updateDelta()
{
	static LARGE_INTEGER start;
	static LARGE_INTEGER last;
	static LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	last = start;
	QueryPerformanceCounter(&start);
	dt = (float)(start.QuadPart - last.QuadPart) / frequency.QuadPart;
	gLastDelta = dt * 1000;
};

// TOTAL_OBJECTS pretty much decides how many drawcalls in a brute force approach.
constexpr int TOTAL_OBJECTS = 1;
// this has to do with how the triangles are spread in the screen, not important.
constexpr int TOTAL_PLACES = 80000;
float xt[TOTAL_PLACES], yt[TOTAL_PLACES], zt[TOTAL_PLACES];


void run() {
	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_MOUSEMOVE)
		{
			POINT pos;
			pos.x = GET_X_LPARAM(msg.lParam);
			pos.y = GET_Y_LPARAM(msg.lParam);
			ClientToScreen(renderer->hwnd, &pos);

			int diffX = pos.x - center.x;
			int diffY = pos.y - center.y;
			camera->rotate(diffX, diffY);
			SetCursorPos(center.x, center.y);
		}
		if (msg.message == WM_KEYDOWN)
		{
			if (msg.wParam == KEY_P)
			{
				shiftStep = !shiftStep;
			}
			camera->startMove(msg.wParam);
		}
		if (msg.message == WM_KEYUP)
		{
			camera->endMove(msg.wParam);
		}
		updateScene();
		renderScene();
	}
}

/*
 update positions of triangles in the screen changing a translation only
*/
void updateScene()
{
	camera->Update(dt);
	static int shift = 0;
	const int size = scene.size();
	for (int i = 0; i < size; i++)
	{
		//scene[i]->rotateMesh(float3({ 0.0f, 0.005f * zt[(i + shift) % (TOTAL_PLACES)], 0.0f }));
		scene[i]->setTranslation(
			{
				xt[((i) + shift) % (TOTAL_PLACES)],
				yt[((i) + shift) % (TOTAL_PLACES)],
				zt[((i) + shift) % (TOTAL_PLACES)],
			}
		);
		scene[i]->Update(camera);
	}
	shift += shiftStep;

	++frameCounter;
	fpsCounter += dt;

	if (fpsCounter > 0.2) {
		fps = (int)round(frameCounter / fpsCounter);
		fpsCounter = frameCounter = 0;
	}

	return;
};


void renderScene()
{
	renderer->clearBuffer(CLEAR_BUFFER_FLAGS::COLOR | CLEAR_BUFFER_FLAGS::DEPTH);
	for (auto m : scene)
	{
		renderer->submit(m);
	}
	renderer->frame();
	renderer->present();
	updateDelta();
	sprintf(gTitleBuff, "DirectX 12 - %.2f", gLastDelta);
	renderer->setWinTitle(gTitleBuff);
}

int initialiseTestbench()
{
	ClientToScreen(renderer->hwnd, &center);

	camera = renderer->makeCamera(SCREEN_WIDTH, SCREEN_HEIGHT);
	float degToRad = M_PI / 180.0;
	float scale = (float)TOTAL_PLACES / 359.9;
	for (int a = 0; a < TOTAL_PLACES; a++)
	{
		xt[a] = 0.8f * cosf(degToRad * ((float)a / scale) * 3.0);
		yt[a] = 0.8f * sinf(degToRad * ((float)a / scale) * 2.0);
		zt[a] = 3.0f * sinf(degToRad * ((float)a / scale));
	};

	float diffuse[4][4] = {
		0.0,1.0,1.0,1.0,
		0.0,1.0,0.0,1.0,
		0.0,0.0,0.0,1.0,
		1.0,0.0,0.0,1.0
	};

	for (int i = 0; i < 4; i++)
	{
		// set material name from text file?
		Material* m = renderer->makeMaterial();

		std::string err;
		m->compileMaterial(err);

		// add a constant buffer to the material, to tint every triangle using this material
		m->addConstantBuffer(DIFFUSE_TINT);
		// no need to update anymore
		// when material is bound, this buffer should be also bound for access.

		m->updateConstantBuffer(diffuse[i], 4 * sizeof(float), DIFFUSE_TINT);
		
		materials.push_back(m);
	}

	// one technique with wireframe
	RenderState* renderState1 = renderer->makeRenderState();
	renderState1->setWireFrame(true);

	// basic technique
	techniques.push_back(renderer->makeTechnique(materials[0], renderState1));
	techniques.push_back(renderer->makeTechnique(materials[1], renderer->makeRenderState()));
	techniques.push_back(renderer->makeTechnique(materials[2], renderer->makeRenderState()));
	techniques.push_back(renderer->makeTechnique(materials[3], renderer->makeRenderState()));

	// create texture
	Texture2D* fatboy = renderer->makeTexture2D();
	fatboy->loadFromFile("../assets/textures/fatboy.png");
	Sampler2D* sampler = renderer->makeSampler2D();
	sampler->setWrap(WRAPPING::REPEAT, WRAPPING::REPEAT);
	fatboy->sampler = sampler;

	textures.push_back(fatboy);
	samplers.push_back(sampler);

	float pos[] = {
		 0.00f,  0.05f,  0.00f, 1.0f,
		 0.05f, -0.05f, -0.05f, 1.0f,
		-0.05f, -0.05f, -0.05f, 1.0f,
		 0.00f,  0.05f,  0.00f, 1.0f,
		 0.00f,  0.05f,  0.00f, 1.0f,
		 0.05f, -0.05f,  0.05f, 1.0f,
		-0.05f, -0.05f,  0.05f, 1.0f,
		 0.00f,  0.05f,  0.00f, 1.0f,
	};
	float nor[] = {
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
	};
	float UV[] = {
		0.50f,  0.00f,
		1.00f,  1.00f,
		0.00f,  1.00f,
		0.50f,  0.00f,
		0.50f,  0.00f,
		1.00f,  1.00f,
		0.00f,  1.00f,
		0.50f,  0.00f,
	};

	DWORD indexList[] = {
		// Front face 
		0, 1, 2,
		0, 2, 3,

		// Back face 
		7, 6, 5,
		7, 5, 4,

		// Right face 
		4, 5, 1,
		4, 1, 0,

		// Left face 
		3, 2, 6,
		3, 6, 7,

		// Top face 
		4, 0, 3,
		4, 3, 7,

		// Bottom face 
		1, 5, 6,
		1, 6, 2,
	};

	// Create a mesh array with 3 basic vertex buffers.
	for (int i = 0; i < TOTAL_OBJECTS; i++) {

		Mesh* m = renderer->makeMesh();

		{
			m->createMeshFromObj(L"../assets/Laptop Keyframes/", 250);
			m->setScale({0.08f, 0.08f, 0.08f});
		}

		// we can create a constant buffer outside the material, for example as part of the Mesh.
		m->wvpBuffer = renderer->makeConstantBuffer(TRANSLATION);

		m->technique = techniques[ i % 4 ];
		if (i % 4 == 2 || i % 4 == 3)
			m->addTexture(textures[0], DIFFUSE_SLOT);

		scene.push_back(m);
	}
	return 0;
}

void shutdown() {
	// shutdown.
	// delete dynamic objects
	delete renderer;
	for (auto m : materials)
	{
		delete(m);
	}
	for (auto t : techniques)
	{
		delete(t);
	}
	for (auto m : scene)
	{
		delete(m);
	};
	
	for (auto s : samplers)
	{
		delete s;
	}

	for (auto t : textures)
	{
		delete t;
	}

	delete camera;
};

int main(int argc, char *argv[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	renderer = Renderer::makeRenderer(Renderer::BACKEND::DX12);
	renderer->initialize(SCREEN_WIDTH, SCREEN_HEIGHT);
	renderer->setWinTitle("Dx12");
	renderer->setClearColor(0.0, 0.1, 0.1, 1.0);
	initialiseTestbench();
	run();
	shutdown();
	return 0;
};
