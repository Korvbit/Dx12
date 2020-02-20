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

using namespace std;
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

void updateDelta()
{
	#define WINDOW_SIZE 10
	static LARGE_INTEGER start;
	static LARGE_INTEGER last;
	static LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	static double avg[WINDOW_SIZE] = { 0.0 };
	static double lastSum = 10.0;
	static int loop = 0;

	last = start;
	QueryPerformanceCounter(&start);
	double deltaTime = (double)(start.QuadPart - last.QuadPart) * 1000 / frequency.QuadPart;
	// moving average window of WINDOWS_SIZE
	lastSum -= avg[loop];
	lastSum += deltaTime;
	avg[loop] = deltaTime;
	loop = (loop + 1) % WINDOW_SIZE;
	gLastDelta = (lastSum / WINDOW_SIZE);
};

// TOTAL_TRIS pretty much decides how many drawcalls in a brute force approach.
constexpr int TOTAL_TRIS = 4;
// this has to do with how the triangles are spread in the screen, not important.
constexpr int TOTAL_PLACES = 40000;
float xt[TOTAL_PLACES], yt[TOTAL_PLACES];


void run() {
	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
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
	/*
	    For each mesh in scene list, update their position 
	*/
	{
		static long long shift = 0;
		const int size = scene.size();
		for (int i = 0; i < size; i++)
		{
			const float4 trans { 
				xt[(int)(float)((200*i) + shift) % (TOTAL_PLACES)], 
				yt[(int)(float)((200*i) + shift) % (TOTAL_PLACES)], 
				i * (-1.0 / TOTAL_PLACES),
				0.0
			};
			scene[i]->txBuffer->setData(&trans, sizeof(trans), scene[i]->technique->getMaterial(), TRANSLATION);
		}
		// just to make them move...
		shift += 1;// max(TOTAL_TRIS / 1000.0, TOTAL_TRIS / 100.0);
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
	sprintf(gTitleBuff, "DirectX 12 - %3.0lf", gLastDelta);
	renderer->setWinTitle(gTitleBuff);
}

int initialiseTestbench()
{
	float degToRad = M_PI / 180.0;
	float scale = (float)TOTAL_PLACES / 359.9;
	for (int a = 0; a < TOTAL_PLACES; a++)
	{
		xt[a] = 0.8f * cosf(degToRad * ((float)a/scale) * 3.0);
		yt[a] = 0.8f * sinf(degToRad * ((float)a/scale) * 2.0);
	};

	// triangle geometry:
	float4 triPos[3] = { { 0.0f,  0.05, 0.0f, 1.0f },{ 0.05, -0.05, 0.0f, 1.0f },{ -0.05, -0.05, 0.0f, 1.0f } };
	float4 triNor[3] = { { 0.0f,  0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f, 0.0f } };
	float2 triUV[3] =  { { 0.5f,  -0.99f },{ 1.49f, 1.1f },{ -0.51, 1.1f } };

	float diffuse[4][4] = {
		0.0,0.0,1.0,1.0,
		0.0,1.0,0.0,1.0,
		0.0,0.0,0.0,1.0,
		1.0,0.0,0.0,1.0
	};

	for (int i = 0; i < 4; i++)
	{
		// set material name from text file?
		Material* m = renderer->makeMaterial("material_" + std::to_string(i));

		std::string err;
		m->compileMaterial(err);

		// add a constant buffer to the material, to tint every triangle using this material
		m->addConstantBuffer(DIFFUSE_TINT_NAME, DIFFUSE_TINT);
		// no need to update anymore
		// when material is bound, this buffer should be also bound for access.

		m->updateConstantBuffer(diffuse[i], 4 * sizeof(float), DIFFUSE_TINT);
		
		materials.push_back(m);
	}

	// one technique with wireframe
	RenderState* renderState1 = renderer->makeRenderState();
	renderState1->setWireFrame(true);
	RenderState* renderState2 = renderer->makeRenderState();

	// basic technique
	techniques.push_back(renderer->makeTechnique(materials[0], renderState1));
	techniques.push_back(renderer->makeTechnique(materials[1], renderState2));
	techniques.push_back(renderer->makeTechnique(materials[2], renderState2));
	techniques.push_back(renderer->makeTechnique(materials[3], renderState2));

	// create texture
	Texture2D* fatboy = renderer->makeTexture2D();
	fatboy->loadFromFile("../assets/textures/fatboy.png");
	Sampler2D* sampler = renderer->makeSampler2D();
	sampler->setWrap(WRAPPING::REPEAT, WRAPPING::REPEAT);
	fatboy->sampler = sampler;

	textures.push_back(fatboy);
	samplers.push_back(sampler);

	// Create a mesh array with 3 basic vertex buffers.
	for (int i = 0; i < TOTAL_TRIS; i++) {

		Mesh* m = renderer->makeMesh();

		//m->createTriangle();
		//m->createQuad();
		m->createCube();

		// we can create a constant buffer outside the material, for example as part of the Mesh.
		m->txBuffer = renderer->makeConstantBuffer(std::string(TRANSLATION_NAME), TRANSLATION);

		m->technique = techniques[ i % 4 ];
		if (i % 4 == 2)
			m->addTexture(textures[0], DIFFUSE_SLOT);

		scene.push_back(m);
	}
	return 0;
}

void shutdown() {
	// shutdown.
	// delete dynamic objects
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
	delete renderer;
};

int main(int argc, char *argv[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	renderer = Renderer::makeRenderer(Renderer::BACKEND::DX12);
	renderer->initialize(800,600);
	renderer->setWinTitle("Dx12");
	renderer->setClearColor(0.0, 0.1, 0.1, 1.0);
	initialiseTestbench();
	run();
	shutdown();
	return 0;
};
