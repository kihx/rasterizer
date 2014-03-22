#ifndef _XTOZERO_H
#define _XTOZERO_H

#ifdef XTOZERO_EXPORTS
#define XTZ_API extern "C" __declspec(dllexport)
#else
#define XTZ_API extern "C" __declspec(dllimport)
#endif

#include "Mesh.h"
#include "Rasterizer.h"

std::unique_ptr<xtozero::CMeshManager> gMeshManager(new xtozero::CMeshManager());

XTZ_API void XtzRenderToBuffer( void* buffer, int width, int height, int dpp );

XTZ_API void XtzClearBuffer( void* buffer, int width, int height, int color );

XTZ_API void XtzLoadMeshFromFile( const char* pfilename );

#endif