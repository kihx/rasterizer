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

EXTERN_FORM DLL_API void __cdecl colorControl( void* pImage, int width, int height, unsigned long clearColor);
