// xtozero.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "xtozero.h"

using namespace xtozero;

XTZ_API void XtzRenderToBuffer( void* buffer, int width, int height, int dpp )
{
	if ( buffer )
	{
		size_t size = width * height * dpp / 8;
		memset( buffer, 100, size );
	}
}

XTZ_API void XtzClearBuffer( void* buffer, int width, int height, int color )
{

}

XTZ_API void XtzLoadMeshFromFile( const char* pfilename )
{
	CMeshManager::GetInstance()->LoadMeshFromFile( pfilename );
}