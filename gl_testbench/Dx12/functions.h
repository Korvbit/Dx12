#pragma once

template <class T> void SafeRelease(T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

template<typename T>
inline void freeContainer(T& p_container)
{
	T empty;
	using std::swap;
	swap(p_container, empty);
}
