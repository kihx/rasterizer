// kihx.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "base.h"
#include "kihx.h"
#include "mesh.h"
#include "texture.h"
#include "render.h"
#include "concommand.h"


static ConsoleVariable grid_scene( "grid_scene", "1" );


static std::vector<std::shared_ptr<IMesh>> g_meshes;


KIHX_API void kiLoadMeshFromFile( const char* filename )
{
	g_meshes.clear();
	for ( int i = 0; i < 4 * 4; ++i )
	{
		g_meshes.emplace_back( kih::CreateMeshFromFile( filename ) );
	}
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

	// Grid arragement
	if ( grid_scene.Bool() )
	{
		Matrix4 matWorld = RenderingDevice::GetInstance()->GetWorldMatrix();

		const size_t Rows = 4;
		float rowFactor = 1.0f / Rows;

		size_t columns = g_meshes.size() / Rows;
		float colFactor = 1.0f / columns;

		for ( size_t row = 0; row < Rows; ++row )
		{
			for ( size_t col = 0; col < columns; ++col )
			{
				Matrix4 matTrans;
				matTrans.Translate( ( ( col + 1 ) * colFactor - colFactor * 2.5f ) * 4.0f,
					( ( row + 1 ) * rowFactor - rowFactor * 2.5f ) * 4.0f,
					( ( row + 1 ) * rowFactor - rowFactor * 2.5f ) * 4.0f );

				Matrix4 matScale;
				matScale.Scaling( 0.3f, 0.3f, 0.3f );

				Matrix4 matNewWorld = matScale * matWorld * matTrans;

				RenderingDevice::GetInstance()->SetWorldMatrix( matNewWorld );

				context->Draw( g_meshes[row * columns + col] );
			}
		}
	}
	else
	{
		context->Draw( g_meshes[0] );
	}
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
