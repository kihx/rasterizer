// kihx.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "base.h"
#include "kihx.h"
#include "mesh.h"
#include "texture.h"
#include "render.h"
#include "concommand.h"




static std::shared_ptr<IMesh> g_mesh;


KIHX_API void kiLoadMeshFromFile( const char* filename )
{
	g_mesh = kih::CreateMeshFromFile( filename );
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
	static std::shared_ptr<Texture> depthStencil = Texture::Create( width, height, ColorFormat::D8S24, NULL );

	context->SetRenderTarget( renderTarget, 0 );
	context->SetDepthStencil( depthStencil );

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

	case 1:
		RenderingDevice::GetInstance()->SetViewMatrix( Matrix4( matrix4x4 ) );
		break;

	case 2:
		RenderingDevice::GetInstance()->SetProjectionMatrix( Matrix4( matrix4x4 ) );
		break;
	
	default:
		LOG_WARNING( "invalid parameter" );
	}
}

KIHX_API void kiExecuteCommand( const char* cmd )
{
	ConsoleCommandExecuter::GetInstance()->Execute( cmd );
}
