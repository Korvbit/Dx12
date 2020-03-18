#pragma once
class RenderState
{
public:
	RenderState();
	virtual ~RenderState() = 0;

	virtual void setWireFrame(bool) = 0;
};

