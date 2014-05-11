// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>



// TODO: reference additional headers your program requires here
#include <cassert>

#define PIXEL_COLOR(r, g, b)  ( ( b << 16 ) + ( g << 8 ) + r )
#define RAND_COLOR() PIXEL_COLOR( ( rand()%255 + 1 ), ( rand()%255 + 1 ), ( rand()%255 + 1 ) )
#define GET_RED(color) (color & 0xFF)
#define GET_GREEN(color) ( (color >> 8) & 0xFF )
#define GET_BULE(color) ( (color >> 16) & 0xFF )