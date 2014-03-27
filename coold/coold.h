#pragma once

#ifdef	__cplusplus
#define		EXTERN_FORM		extern "C"
#else
#define		EXTERN_FORM
#endif

#ifdef DLLEXPORTS
#define DLL_API _declspec(dllexport)
#else
#define DLL_API _declspec(dllimport)
#endif

#include "CoolD_Type.h"

EXTERN_FORM DLL_API void __cdecl coold_LoadMeshFromFile( const Dchar* filename );
EXTERN_FORM DLL_API void __cdecl coold_RenderToBuffer( void* buffer, Dint width, Dint height, Dint bpp );
EXTERN_FORM DLL_API void __cdecl coold_SetTransform(Dint transformType, const Dfloat* matrix4x4);