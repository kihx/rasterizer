// xtozero.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "xtozero.h"

XTZ_API void ClearBuffer( void* buffer, int width, int height, int colorDepth )
{
	if ( buffer )
	{
		size_t size = width * height * colorDepth;
		memset( buffer, 100, size );
	}
}