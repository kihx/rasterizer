// kihx.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "base.h"
#include "kihx.h"
#include "mesh.h"
#include "render.h"





static std::shared_ptr<kih::Mesh> g_mesh;


KIHX_API void kiLoadMeshFromFile( const char* filename )
{
	g_mesh = kih::Mesh::CreateFromFile( filename );
}

KIHX_API void kiRenderToBuffer( void* buffer, int width, int height, int bpp )
{
	if ( buffer == nullptr )
	{
		return;
	}

	// clear buffer
	//size_t bufferSize = width * height * ( bpp / 8 );
	//::memset( buffer, 255, bufferSize );

	kih::ColorFormat colorFormat = kih::GetSuitableColorFormatFromBpp( bpp );

	__UNDONE( temporal testing code );
	static std::shared_ptr<kih::RenderingContext> context = kih::RenderingDevice::GetInstance()->CreateRenderingContext();
	static std::shared_ptr<kih::Texture> renderTarget = kih::Texture::Create( width, height, colorFormat, buffer );

	context->SetRenderTarget( renderTarget, 0 );

	context->Clear( 0, 0, 0, 255 );

	context->Draw( g_mesh );
}
