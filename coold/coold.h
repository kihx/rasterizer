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

#include "Data/CoolD_Type.h"

EXTERN_FORM DLL_API Dvoid __cdecl coold_LoadMeshFromFile( const Dchar* filename );
EXTERN_FORM DLL_API Dvoid __cdecl coold_RenderToBuffer( Dvoid* buffer, Dint width, Dint height, Dint bpp );
EXTERN_FORM DLL_API Dvoid __cdecl coold_SetTransform(Dint transformType, const Dfloat* matrix4x4);
EXTERN_FORM DLL_API Dvoid __cdecl coold_SetViewFactor(Dfloat* eye, Dfloat* lookat, Dfloat* up);
EXTERN_FORM DLL_API Dvoid __cdecl coold_SetPerspectiveFactor(Dfloat fovY, Dfloat aspect, Dfloat zn, Dfloat zf);
EXTERN_FORM DLL_API Dvoid __cdecl coold_DetachModuleClear();