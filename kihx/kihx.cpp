// kihx.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "base.h"
#include "kihx.h"
#include "mesh.h"
#include "texture.h"
#include "render.h"
#include "concommand.h"

#include <array>



// ConsoleVariables
static ConsoleVariable grid_scene( "grid_scene", "0" );
static ConsoleVariable context_concurrency( "context_concurrency", "1" );


// Resources
static std::vector<std::shared_ptr<IMesh>> g_meshes;



KIHX_API void kiLoadMeshFromFile( const char* filename )
{
	g_meshes.clear();
	for ( int i = 0; i < 4 * 4; ++i )
	{
		g_meshes.emplace_back( kih::CreateMeshFromFile( filename ) );
	}
}

void DrawGridScene( std::shared_ptr<RenderingContext> context, const Matrix4& matWorld )
{
	Assert( context );

	ConstantBuffer& cbuffer = context->GetSharedConstantBuffer();

	const size_t Rows = 4;
	size_t columns = g_meshes.size() / Rows;	
	float rowFactor = 1.0f / Rows;
	float colFactor = 1.0f / columns;
	for ( size_t row = 0; row < Rows; ++row )
	{
		for ( size_t col = 0; col < columns; ++col )
		{
			// world transform
			Matrix4 matTrans;
			matTrans.Translate( ( ( col + 1 ) * colFactor - colFactor * 2.5f ) * 4.0f,
				( ( row + 1 ) * rowFactor - rowFactor * 2.5f ) * 4.0f,
				( ( row + 1 ) * rowFactor - rowFactor * 2.5f ) * 4.0f );

			Matrix4 matScale;
			matScale.Scaling( 0.3f, 0.3f, 0.3f );

			Matrix4 matNewWorld = matScale * matWorld * matTrans;
			cbuffer.SetMatrix4( ConstantBuffer::WorldMatrix, matNewWorld );

			// various color
			cbuffer.SetVector4( ConstantBuffer::DiffuseColor, Vector4( max( 0.2f, matTrans.A41 * 0.5f + 0.8f ), max( 0.4f, matTrans.A42 * 0.5f + 0.6f ), matTrans.A43 * 0.5f + 1.0f, 1.0f ) );

			context->Draw( g_meshes[row * columns + col] );
		}
	}
}

void DrawGridSceneInParallel( std::vector<std::shared_ptr<RenderingContext>> contexts, const Matrix4& matWorld )
{
	const size_t Rows = 4;
	size_t numMeshes = g_meshes.size();
	size_t columns = numMeshes / Rows;
	float rowFactor = 1.0f / Rows;
	float colFactor = 1.0f / columns;

	//using FuncPreRender = std::function<void( std::shared_ptr<RenderingContext> context, const std::shared_ptr<IMesh>& mesh, size_t index )>;
	RenderingContext::FuncPreRender funcPreRender(
		[=]( std::shared_ptr<RenderingContext> context, const std::shared_ptr<IMesh>& mesh, size_t index )
		{
			ConstantBuffer& cbuffer = context->GetSharedConstantBuffer();

			// world transform
			size_t row = kih::Clamp<size_t>( index / Rows, 0, Rows - 1 );
			size_t col = kih::Clamp<size_t>( index - row * columns, 0, columns - 1 );

			Matrix4 matTrans;
			matTrans.Translate( ( ( col + 1 ) * colFactor - colFactor * 2.5f ) * 4.0f,
				( ( row + 1 ) * rowFactor - rowFactor * 2.5f ) * 4.0f,
				( ( row + 1 ) * rowFactor - rowFactor * 2.5f ) * 4.0f );

			Matrix4 matScale;
			matScale.Scaling( 0.3f, 0.3f, 0.3f );

			Matrix4 matNewWorld = matScale * matWorld * matTrans;			
			cbuffer.SetMatrix4( ConstantBuffer::WorldMatrix, matNewWorld );

			// various color
			cbuffer.SetVector4( ConstantBuffer::DiffuseColor, Vector4( max( 0.2f, matTrans.A41 * 0.5f + 0.8f ), max( 0.4f, matTrans.A42 * 0.5f + 0.6f ), matTrans.A43 * 0.5f + 1.0f, 1.0f ) );
		}
	);

	RenderingContext::DrawInParallel( contexts, g_meshes, funcPreRender );
}

KIHX_API void kiRenderToBuffer( void* buffer, int width, int height, int bpp )
{
	if ( buffer == nullptr )
	{
		return;
	}
	
	__UNDONE( temporal testing code );

	// Create color and depth stencil buffers.
	ColorFormat colorFormat = kih::GetSuitableColorFormatFromBpp( bpp );
	static std::shared_ptr<Texture> renderTarget = Texture::Create( width, height, colorFormat, buffer );
	static std::shared_ptr<Texture> depthStencil = Texture::Create( width, height, ColorFormat::D32F, NULL );

	// Create rendering contexts as many of RenderingContext::ThreadConcurrency.
	static std::array<std::shared_ptr<RenderingContext>, RenderingContext::ThreadConcurrency> contexts;
	if ( contexts[0] == nullptr )
	{
		int index = 0;
		LoopUnroll<RenderingContext::ThreadConcurrency>::Work( 
			[&index]() 
			{ 
				auto context = RenderingDevice::GetInstance()->CreateRenderingContext(); 
				context->SetRenderTarget( renderTarget, 0 );		// Assume that we have only one RT.
				context->SetDepthStencil( depthStencil );
				contexts[index++] = context;
			} 
		);
	}
	
	auto context = contexts[0];

	context->Clear( 0, 0, 0, 255 );

	// Draw the world.
	const Matrix4& matWorld = RenderingDevice::GetInstance()->GetWorldMatrix();

	// Grid arragement
	if ( grid_scene.Bool() )
	{
		// threading or not
		if ( context_concurrency.Int() <= 1 )
		{
			DrawGridScene( context, matWorld );
		}
		else
		{
			// Copy array contexts to vector one.
			std::vector<std::shared_ptr<RenderingContext>> concurrencyContexts;
			concurrencyContexts.reserve( contexts.size() );			
			for ( auto context : contexts )
			{
				concurrencyContexts.emplace_back( context );
			}

			DrawGridSceneInParallel( concurrencyContexts, matWorld );
		}
	}
	else
	{
		context->GetSharedConstantBuffer().SetMatrix4( ConstantBuffer::WorldMatrix, matWorld );
		context->GetSharedConstantBuffer().SetVector4( ConstantBuffer::DiffuseColor, Vector4( 1.0f, 1.0f, 1.0f, 1.0f ) );
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
