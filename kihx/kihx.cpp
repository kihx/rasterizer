// kihx.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "base.h"
#include "kihx.h"
#include "mesh.h"
#include "texture.h"
#include "render.h"





static std::shared_ptr<Mesh> g_mesh;


KIHX_API void kiLoadMeshFromFile( const char* filename )
{
	g_mesh = Mesh::CreateFromFile( filename );
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

	ColorFormat colorFormat = kih::GetSuitableColorFormatFromBpp( bpp );

	__UNDONE( temporal testing code );
	static std::shared_ptr<RenderingContext> context = RenderingDevice::GetInstance()->CreateRenderingContext();
	static std::shared_ptr<Texture> renderTarget = Texture::Create( width, height, colorFormat, buffer );

	context->SetRenderTarget( renderTarget, 0 );

	context->Clear( 0, 0, 0, 255 );

	context->Draw( g_mesh );
}

KIHX_API void kiSetTransform( int transformType, const float* matrix4x4 )
{
	switch ( transformType )
	{
	case 0:
		RenderingDevice::GetInstance()->SetWorldMatrix( Matrix4( matrix4x4 ) );
		break;

	case 2:
		RenderingDevice::GetInstance()->SetViewMatrix( Matrix4( matrix4x4 ) );
		break;

	case 1:
		RenderingDevice::GetInstance()->SetProjectionMatrix( Matrix4( matrix4x4 ) );
		break;
	
	default:
		LOG_WARNING( "invalid parameter" );
	}
}
