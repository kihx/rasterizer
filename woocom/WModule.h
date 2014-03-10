#pragma once

#ifdef WMODULE_API

#else
#define WMODULE_API extern "C" __declspec(dllimport)
#endif


typedef unsigned char uchar;

WMODULE_API bool Initialize( void* buffer, int width, int height, int colorDepth);
WMODULE_API void Clear( uchar r, uchar g, uchar b);
