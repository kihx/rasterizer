// kihx.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "base.h"
#include "concommand.h"
#include "kihx.h"
#include "mathlib.h"
#include "mesh.h"
#include "profiler.h"
#include "render.h"
#include "stream.h"
#include "texture.h"

#include <array>



// ConsoleVariables
static ConsoleVariable grid_scene( "grid_scene", "0" );
static ConsoleVariable grid_size( "grid_size", "5" );

static ConsoleVariable model_scale( "model_scale", "0.3" );

static ConsoleVariable concurrency( "concurrency", "4" );


// Resources
FORCEINLINE StlVector<std::shared_ptr<IMesh>>& GetMeshes()
{
	static StlVector<std::shared_ptr<IMesh>> g_meshes;
	return g_meshes;
}



KIHX_API void kiLoadMeshFromFile( const char* filename )
{
	auto& meshes = GetMeshes();
	meshes.clear();

	meshes.emplace_back( kih::CreateMeshFromFile( filename ) );
	if ( meshes[0] == nullptr )
	{
		meshes.clear();
		return;
	}

	for ( int i = 0; i < 6 * 6 - 1; ++i )
	{
		auto mesh = meshes[0]->Clone();
		if ( mesh )
		{
			meshes.push_back( mesh );
		}
		else
		{
			meshes.emplace_back( kih::CreateMeshFromFile( filename ) );
		}
	}
}

void DrawGridScene( std::shared_ptr<RenderingContext> context, const Matrix4& matWorld )
{
	Assert( context );

	ConstantBuffer& cbuffer = context->GetSharedConstantBuffer();

	float scale = model_scale.Float();

	size_t columns = grid_size.Int();
	float rowFactor = 1.0f / grid_size.Int();
	float colFactor = 1.0f / columns;
	for ( int row = 0; row < grid_size.Int(); ++row )
	{
		for ( size_t col = 0; col < columns; ++col )
		{
			// world transform
			Matrix4 matTrans;
			matTrans.Translate( ( ( col + 1 ) * colFactor - colFactor * ( columns * 0.5f ) ) * columns,
				( ( row + 1 ) * rowFactor - rowFactor * ( columns * 0.5f ) ) * columns,
				( ( row + 1 ) * rowFactor - rowFactor * ( columns * 0.5f ) ) * columns );

			Matrix4 matScale;
			matScale.Scaling( scale, scale, scale );

			Matrix4 matNewWorld = matScale * matWorld * matTrans;
			cbuffer.SetMatrix4( ConstantBuffer::WorldMatrix, matNewWorld );

			// various color
			cbuffer.SetVector4( ConstantBuffer::DiffuseColor, Vector4( max( 0.2f, matTrans.A41 * 0.5f + 0.8f ), max( 0.4f, matTrans.A42 * 0.5f + 0.6f ), matTrans.A43 * 0.5f + 1.0f, 1.0f ) );

			context->Draw( GetMeshes()[row * columns + col] );
		}
	}
}

void DrawGridSceneInParallel( StlVector<std::shared_ptr<RenderingContext>>& contexts, const Matrix4& matWorld )
{
	float scale = model_scale.Float();

	size_t rows = grid_size.Int();
	size_t columns = grid_size.Int();
	float rowFactor = 1.0f / grid_size.Int();
	float colFactor = 1.0f / columns;

	//using FuncPreRender = std::function<void( std::shared_ptr<RenderingContext> context, const std::shared_ptr<IMesh>& mesh, size_t index )>;
	RenderingContext::FuncPreRender funcPreRender(
		[=]( std::shared_ptr<RenderingContext> context, size_t index )
		{
			ConstantBuffer& cbuffer = context->GetSharedConstantBuffer();

			// world transform
			size_t row = kih::Clamp<size_t>( index / rows, 0, rows - 1 );
			size_t col = kih::Clamp<size_t>( index - row * columns, 0, columns - 1 );

			Matrix4 matTrans;
			matTrans.Translate( ( ( col + 1 ) * colFactor - colFactor * ( columns * 0.5f ) ) * columns,
				( ( row + 1 ) * rowFactor - rowFactor * ( columns * 0.5f ) ) * columns,
				( ( row + 1 ) * rowFactor - rowFactor * ( columns * 0.5f ) ) * columns );

			Matrix4 matScale;
			matScale.Scaling( scale, scale, scale );

			Matrix4 matNewWorld = matScale * matWorld * matTrans;			
			cbuffer.SetMatrix4( ConstantBuffer::WorldMatrix, matNewWorld );

			// various color
			cbuffer.SetVector4( ConstantBuffer::DiffuseColor, Vector4( max( 0.2f, matTrans.A41 * 0.5f + 0.8f ), max( 0.4f, matTrans.A42 * 0.5f + 0.6f ), matTrans.A43 * 0.5f + 1.0f, 1.0f ) );
		}
	);

	RenderingContext::DrawInParallel( contexts, GetMeshes(), grid_size.Int() * grid_size.Int(), funcPreRender );
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
	static std::shared_ptr<UnorderedAccessView<OutputMergerInputStream>> omUAV = std::make_shared<UnorderedAccessView<OutputMergerInputStream>>( width * height * 2 );

	// Create rendering contexts as many of Thread::HardwareConcurrency.
	const int Concurrency = 8;	// HACK
	static std::array<std::shared_ptr<RenderingContext>, Concurrency> contexts;
	if ( contexts[0] == nullptr )
	{
		int index = 0;
		LoopUnroll<Concurrency>::Work( 
			[&index]() 
			{ 
				auto context = RenderingDevice::GetInstance()->CreateRenderingContext();
				context->SetViewport( 0, 0, static_cast<unsigned short>( renderTarget->Width() ), static_cast<unsigned short>( renderTarget->Height() ) );
				context->SetFixedPipelineMode( false );
				
				context->SetRenderTarget( renderTarget, 0 );
				context->SetDepthStencil( depthStencil );

				contexts[index++] = context;
			} 
		);
	}
	
	auto context = contexts[0];

	context->Clear( 0, 0, 0, 255 );
	context->SetUnorderedAccessView( nullptr );

	// Draw the world.
	const Matrix4& matWorld = RenderingDevice::GetInstance()->GetWorldMatrix();

	// Grid arragement
	if ( grid_scene.Bool() )
	{
		// threading or not
		if ( concurrency.Int() <= 1 )
		{
			DrawGridScene( context, matWorld );
		}
		else
		{
			// Copy array contexts to vector one.
			StlVector<std::shared_ptr<RenderingContext>> concurrencyContexts;
			concurrencyContexts.resize( concurrency.Int() - 1 );		// the other thread is for resolve.	
			for ( size_t i = 0; i < concurrencyContexts.size(); ++i )
			{
				contexts[i]->SetUnorderedAccessView( omUAV );
				concurrencyContexts[i] = contexts[i];
			}

			DrawGridSceneInParallel( concurrencyContexts, matWorld );
		}
	}
	else
	{
		context->GetSharedConstantBuffer().SetMatrix4( ConstantBuffer::WorldMatrix, matWorld );
		context->GetSharedConstantBuffer().SetVector4( ConstantBuffer::DiffuseColor, Vector4( 1.0f, 1.0f, 1.0f, 1.0f ) );
		context->Draw( GetMeshes()[0] );
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
