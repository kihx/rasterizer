#ifndef _XTOZERO_H
#define _XTOZERO_H

#ifndef DLL_EXPORT
#define XTZ_API __declspec(dllexport)
#else
#define XTZ_API __declspec(dllimport)
#endif

XTZ_API void ClearBuffer( void* buffer, int width, int height, int colorDepth );

#endif