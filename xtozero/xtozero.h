#ifndef _XTOZERO_H
#define _XTOZERO_H

#ifndef DLL_EXPORT
#define XTZ_API __declspec(dllexport)
#else
#define XTZ_API __declspec(dllimport)
#endif

#include "Mesh.h"

XTZ_API void XtzClearBuffer( void* buffer, int width, int height, int colorDepth );

XTZ_API void XtzLoadMeshFromFile( const char* pfilename );

#endif