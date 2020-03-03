#pragma once
#include <string>
#include "Sampler2D.h"


class Texture2D
{
public:
	Texture2D();
	virtual ~Texture2D();

	// returns 0 if texture was loaded.
	virtual int loadFromFile(std::string filename) = 0;

	// if no sampler is set here, a default sampler should be used.
	Sampler2D* sampler = nullptr;
};

