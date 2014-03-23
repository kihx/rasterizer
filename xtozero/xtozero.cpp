// xtozero.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "xtozero.h"

using namespace xtozero;

XTZ_API void XtzRenderToBuffer( void* buffer, int width, int height, int dpp )
{
	if ( buffer )
	{
		CRasterizer rasterizer( gMeshManager->LoadRecentMesh() );

		size_t size = dpp / 8;

		BYTE* buf = static_cast<BYTE*>(buffer);
		unsigned int color = PIXEL_COLOR( 0, 0, 0 );

		//Output Merger 구현후 수정
		auto output = rasterizer.Getoutput();

		for ( int i = 0; i < height; ++i )
		{
			for ( int j = 0; j < width; ++j )
			{
				memcpy_s( buf + ((width * i) + j) * size,
					size,
					&color,
					size );
			}
		}

		for ( auto iter = output.begin(); iter != output.end(); ++iter )
		{
			memcpy_s( buf + ((width * iter->m_y) + iter->m_x) * size,
				size,
				&iter->m_color,
				size );
		}
	}
}

XTZ_API void XtzClearBuffer( void* buffer, int width, int height, int color )
{

}

XTZ_API void XtzLoadMeshFromFile( const char* pfilename )
{
	gMeshManager->LoadMeshFromFile( pfilename );
}