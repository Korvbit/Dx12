#pragma once
class RenderState
{
public:
	RenderState();
	~RenderState();

	virtual void setWireFrame(bool) = 0;
};

