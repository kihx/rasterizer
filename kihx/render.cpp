#include "stdafx.h"
#include "buffer.h"
#include "concommand.h"
#include "matrix.h"
#include "mesh.h"
#include "profiler.h"
#include "stage.h"
#include "stream.h"
#include "render.h"
#include "texture.h"
#include "threading.h"
#include "vector.h"

#include <thread>


namespace kih
{	
	/* class RenderingContext
	*/
	RenderingContext::RenderingContext( size_t numRenderTargets ) :
		m_inputAssembler( std::make_shared<InputAssembler>( this ) ),
		m_vertexShader( std::make_shared<VertexShader>( this ) ),
		m_rasterizer( std::make_shared<Rasterizer>( this ) ),
		m_pixelShader( std::make_shared<PixelShader>( this ) ),
		m_outputMerger( std::make_shared<OutputMerger>( this ) ),
		m_depthFunc( DepthFunc::None ),
		m_depthWritable( false ),
		m_cullMode( CullMode::None ),
		m_fixedPipelineMode( false )
	{
		m_renderTargets.resize( numRenderTargets );
	}

	void RenderingContext::Clear( byte r, byte g, byte b, byte a, float z, int stencil )
	{
		//unsigned int color = ( r | ( ((unsigned short) g) << 8 ) | ( ((unsigned int) b) << 16 )) | ( ((unsigned int) a) << 24 );

		a = 255;	// FIXME

		// for each render target
		size_t num = m_renderTargets.size();
		for ( size_t i = 0; i < num; ++i )
		{
			std::shared_ptr<Texture> rt = m_renderTargets[i];
			if ( rt == nullptr )
			{
				continue;
			}

			LockGuardPtr<Texture> guard( rt );
			void* ptr = guard.Ptr();
			if ( ptr == nullptr )
			{
				continue;
			}

			Assert( ptr );

			int width = rt->Width();
			int height = rt->Height();

			if ( r == g && g == b )
			{
				int size = width * height * GetBytesPerPixel( rt->Format() );
				memset( ptr, r, size );
				continue;
			}
					
			for ( int h = 0; h < height; ++h )
			{
				for ( int w = 0; w < width; ++w )
				{
					rt->WriteTexel( w, h, r, g, b );
				}
			}
		}

		// depth-stencil
		std::shared_ptr<Texture> ds = m_depthStencil;
		if ( ds == nullptr )
		{
			return;
		}

		LockGuardPtr<Texture> guard( ds );
		void* ptr = guard.Ptr();
		if ( ptr == nullptr )
		{
			return;
		}

		Assert( ptr );
		
		int width = ds->Width();
		int height = ds->Height();
		int bytePerPixel = GetBytesPerPixel( ds->Format() );

		switch ( ds->Format() )
		{
		case ColorFormat::D8S24:
			{
				// FIXME: stencil value
				memset( ptr, Float_ToByte( z ), width * height * bytePerPixel );
				Unused( stencil );
			}
			break;

		case ColorFormat::D32F:
			{
				float* fp = reinterpret_cast< float* >( ptr );
				Assert( fp );
				int size = width * height;
				for ( int i = 0; i < size; ++i )
				{
					fp[i] = z;
				}
			}
			break;

		default:
			throw std::invalid_argument( "invalid depth-stencil format" );
		}
	}

	void RenderingContext::Draw( const std::shared_ptr<IMesh>& mesh )
	{
		if ( mesh == nullptr )
		{
			return;
		}

		RenderingDevice* pDevice = RenderingDevice::GetInstance();
		Assert( pDevice );
		Assert( m_inputAssembler );
		Assert( m_vertexShader );
		Assert( m_rasterizer );
		Assert( m_pixelShader );
		Assert( m_outputMerger );

		// Set a constant buffer for WVP transform.
		ConstantBuffer& cbuffer = GetSharedConstantBuffer();
		const Matrix4& w = cbuffer.GetMatrix4( ConstantBuffer::WorldMatrix );
		Matrix4 wv = w * pDevice->GetViewMatrix();
		Matrix4 wvp = wv * pDevice->GetProjectionMatrix();
		cbuffer.SetMatrix4( ConstantBuffer::WVPMatrix, wvp );

		// Draw primitives here.
		if ( mesh->GetPrimitiveType() == PrimitiveType::Undefined )
		{
			if ( const IrregularMesh* pMesh = dynamic_cast< const IrregularMesh* >( mesh.get() ) )
			{
				for ( size_t face = 0; face < pMesh->NumFaces(); ++face )
				{
					const unsigned char* color = pMesh->GetFaceColor( face );
					cbuffer.SetVector4( ConstantBuffer::DiffuseColor, Vector4( static_cast<byte>( color[0] / 255.0f ), 
																				static_cast<byte>( color[1] / 255.0f ), 
																				static_cast<byte>( color[2] / 255.0f ), 
																				static_cast<byte>( color[3] / 255.0f ) ) );

					m_inputAssembler->SetFaceIndex( face );
					SetCullMode( CullMode::None );
					DrawInternal( mesh, GetPrimitiveTypeFromNumberOfVertices( pMesh->NumVerticesInFace( face ) ) );
				}
			}
		}
		else
		{
			SetCullMode( CullMode::CCW );
			DrawInternal( mesh );
		}
	}

	void RenderingContext::DrawInParallel( StlVector<std::shared_ptr<RenderingContext>>& contexts, const StlVector<std::shared_ptr<IMesh>>& meshes, int meshCount, FuncPreRender funcPreRender )
	{
		if ( meshes.empty() )
		{
			return;
		}

		if ( contexts.empty() )
		{
			LOG_WARNING( "no contexts" );
			return;
		}

		// Validate the specified rendering contexts.
		for ( auto context : contexts )
		{
			if ( context == nullptr )
			{
				LOG_WARNING( "forbidden null context usage" );
				return;
			}
		}

		if ( meshes.size() < static_cast<size_t>( meshCount ) )
		{
			LOG_WARNING( "invalid mesh count" );
			return;
		}

		// If the number of meshes is less than 4, 
		// draw all meshes on the first context without parallel processing.
		if ( meshCount < Thread::HardwareConcurrency() )
		{
			auto context = contexts[0];
			for ( const auto& mesh : meshes )
			{
				context->Draw( mesh );
			}
			return;
		}


		RenderingDevice* pDevice = RenderingDevice::GetInstance();
		Assert( pDevice );

		size_t numContexts = contexts.size();

		// Note that the world matrix would be multiplied just before drawing a mesh
		// because the world matrix of a mesh is different each other.
		// So premultiply view-projection matrices only.
		Matrix4 viewProj = pDevice->GetViewMatrix() * pDevice->GetProjectionMatrix();

		// Thread objects.
		ThreadPool* threadPool = ThreadPool::GetInstance();
		Atomic<int> workIndex( 0 );

		for ( size_t c = 0; c < numContexts; ++c )
		{
			auto context = contexts[c];
			Assert( context );

			auto task = std::bind(
				[=, &workIndex]( std::shared_ptr<RenderingContext> context, Matrix4 viewProj, FuncPreRender funcPreRender )
				{
					SCOPE_PROFILE_BEGIN( "RenderTask" );

					ConstantBuffer& cbuffer = context->GetSharedConstantBuffer();

					// Until the last index...
					while ( workIndex.Value() < meshCount )
					{
						// Prefech the workIndex to avoid changing the value on other thread, and then increment the workIndex.
						int index = workIndex.FetchAndIncrement();

						// Check the index again to guarantee atomic.
						// FIXME: Is this necessary?
						if ( index >= meshCount )
						{
							return;
						}

						// Call the PreRender functor.
						funcPreRender( context, index );

						// Now compute the final WVP matrix of this mesh.
						Matrix4 wvp = cbuffer.GetMatrix4( ConstantBuffer::WorldMatrix ) * viewProj;
						cbuffer.SetMatrix4( ConstantBuffer::WVPMatrix, wvp );

						// Draw the mesh.
						context->Draw( meshes[index] );
					}

					SCOPE_PROFILE_END();
				},
				context,
				viewProj,
				funcPreRender );

			threadPool->Queue( task );
		}


		// Join all rendering threads.
		SCOPE_PROFILE_BEGIN( "JoinRendering" );
		threadPool->WaitForAllTasks();
		SCOPE_PROFILE_END();

			
		//// Resolve the UAV to display using the main context.
		//// We assume that rendering order is independent.
		//SCOPE_PROFILE_BEGIN( "ResolveUnorderedAccessView " );
		//contexts[0]->ResolveUnorderedAccessViews( contexts );
		//SCOPE_PROFILE_END();
	}

	bool RenderingContext::SetRenderTarget( std::shared_ptr<Texture> texture, size_t index )
	{
		if ( index < 0 || index >= m_renderTargets.size() )
		{
			return false;
		}

		if ( texture )
		{
			if ( ColorFormat_IsDepthStencil( texture->Format() ) )
			{
				Assert( 0 && "invalid parameter" );
				return false;
			}

			m_renderTargets[index] = texture;
			return true;
		}

		return false;
	}

	bool RenderingContext::SetDepthStencil( std::shared_ptr<Texture> texture )
	{
		if ( texture )
		{
			if ( ColorFormat_IsColor( texture->Format() ) )
			{
				Assert( 0 && "invalid parameter" );
				return false;
			}
		}

		m_depthStencil = texture;
		return true;
	}
	
	void RenderingContext::SetUnorderedAccessView( std::shared_ptr<UnorderedAccessView<OutputMergerInputStream>> omUAV )
	{
		m_outputMerger->SetUnorderedAccessView( omUAV );
	}

	void RenderingContext::SetDepthWritable( bool writable )
	{
		m_depthWritable = writable;
	}

	void RenderingContext::SetDepthFunc( DepthFunc func )
	{
		m_depthFunc = func;
	}

	void RenderingContext::DrawInternal( const std::shared_ptr<IMesh>& mesh, PrimitiveType primitiveType /*= PrimitiveType::Triangles */ )
	{
		Assert( m_inputAssembler );
		Assert( m_vertexShader );
		Assert( m_rasterizer );
		Assert( m_pixelShader );
		Assert( m_outputMerger );

		// input assembler stage
		if ( IsFixedPipelineMode() )
		{
			m_inputAssembler->BindFixedPipelineOutputStreamSource( m_vertexShader->OutputStreamSource() );
		}
		std::shared_ptr<VertexShaderInputStream> vpInput = m_inputAssembler->Process( mesh );
		//printf( "\nVertexShaderInputStream Size: %d\n", vpInput->Size() );

		// vertex processor stage
		std::shared_ptr<RasterizerInputStream> raInput = m_vertexShader->Process( vpInput );
		//printf( "RasterizerInputStream Size: %d\n", raInput->Size() );

		// rasterizer stage
		raInput->SetPrimitiveType( primitiveType );
		m_rasterizer->SetCullMode( GetCullMode() );
		std::shared_ptr<PixelShaderInputStream> ppInput = m_rasterizer->Process( raInput );
		//printf( "PixelShaderInputStream Size: %d\n", ppInput->Size() );

		// pixel processor stage
		std::shared_ptr<OutputMergerInputStream> omInput = m_pixelShader->Process( ppInput );
		//printf( "OutputMergerInputStream Size: %d\n", 0 );
		
		// output merger stage
		m_outputMerger->Process( omInput );
		// Note that the output stream of the output merger is meaningless
		// because the output merger directly write data on render targets and a depth-stencil buffer, or a unordered resource view.
	}

	void RenderingContext::ResolveUnorderedAccessViews( StlVector<std::shared_ptr<RenderingContext>>& contexts )
	{
		Assert( m_outputMerger );

		for ( const auto& context : contexts )
		{
			auto uav = context->m_outputMerger->GetUnorderedAccessView();
			if ( uav == nullptr )
			{
				continue;
			}

			m_outputMerger->Resolve( uav->GetStreamSource() );
			uav->Clear();
		}
	}


	/* RenderingDevice
	*/
	void RenderingDevice::SetWorldMatrix( const Matrix4& m )
	{
		m_worldMatrix = m;

		for ( auto& context : m_renderingContexts )
		{
			context->GetSharedConstantBuffer().SetMatrix4( ConstantBuffer::WorldMatrix, m );
		}
	}

	void RenderingDevice::SetViewMatrix( const Matrix4& m )
	{
		m_viewMatrix = m;

		for ( auto& context : m_renderingContexts )
		{
			context->GetSharedConstantBuffer().SetMatrix4( ConstantBuffer::ViewMatrix, m );
		}
	}

	void RenderingDevice::SetProjectionMatrix( const Matrix4& m )
	{
		m_projMatrix = m;

		for ( auto& context : m_renderingContexts )
		{
			context->GetSharedConstantBuffer().SetMatrix4( ConstantBuffer::ProjectionMatrix, m );
		}
	}

	std::shared_ptr<RenderingContext> RenderingDevice::CreateRenderingContext()
	{
		auto context = std::make_shared<RenderingContext>( 1 /* the number of render targets */ );
		
		// render states initialization
		context->SetDepthFunc( DepthFunc::LessEqual );
		context->SetDepthWritable( true );

		m_renderingContexts.emplace_back( context );
		return context;
	}
}



/* cheats
*/
DEFINE_COMMAND( depth_test_on )
{
	auto context = RenderingDevice::GetInstance()->GetRenderingContext( 0 );
	Assert( context );
	context->SetDepthFunc( DepthFunc::LessEqual );
	context->SetDepthWritable( true );
}

DEFINE_COMMAND( depth_test_off )
{
	auto context = RenderingDevice::GetInstance()->GetRenderingContext( 0 );
	Assert( context );
	context->SetDepthFunc( DepthFunc::None );
	context->SetDepthWritable( false );
}
