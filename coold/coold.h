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

EXTERN_FORM DLL_API void __cdecl coold_LoadMeshFromFile( const char* filename );
EXTERN_FORM DLL_API void __cdecl coold_ClearColorBuffer( void* pImage, int width, int height, unsigned long clearColor);
EXTERN_FORM DLL_API void __cdecl coold_RenderToBuffer( void* buffer, int width, int height, int bpp );