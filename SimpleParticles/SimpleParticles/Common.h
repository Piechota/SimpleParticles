#pragma once

#include <Windows.h>
#include <ctime>

template<typename T>
inline void SafeRelease(T& ptr)
{
	if (ptr != nullptr)
	{
		ptr->Release();
		ptr = nullptr;
	}
}