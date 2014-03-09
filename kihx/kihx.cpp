// kihx.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "kihx.h"
#include "mesh.h"

#include <memory>


using namespace std;
using namespace kihx;


KIHX_API void kiLoadMeshFromFile( const char* filename )
{
	shared_ptr<Mesh> mesh( Mesh::CreateFromFile( filename ) );
}

KIHX_API void kiRenderToBuffer( void* buffer, int width, int height, int bpp )
{
	if ( buffer == NULL )
	{
		return;
	}

	size_t bufferSize = width * height * (bpp / 8);
		
	::memset( buffer, 255, bufferSize );
}
