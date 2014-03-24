#pragma once


#ifdef WMODULE_API


#else
#define WMODULE_API extern "C" __declspec(dllimport)
#endif


WMODULE_API void WLoadMesh(const char* filename);
WMODULE_API void WRender(void* buffer, int width, int height, int bpp);
WMODULE_API void WClear(void* pImage, int width, int height, unsigned long clearColor);
