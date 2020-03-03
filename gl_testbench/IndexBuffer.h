#pragma once
class IndexBuffer
{
public:


	IndexBuffer() {};
	virtual ~IndexBuffer() {}
	virtual void setData(const void* data, size_t size, size_t offset) = 0;
	void incRef() { refs++; };
	void decRef() { if (refs > 0) refs--; };
	inline unsigned int refCount() { return refs; };
protected:
private:
	//cheap ref counting
	unsigned int refs = 0;
};

