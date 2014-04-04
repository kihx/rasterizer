#include "stdafx.h"
#include "render.h"
#include "mesh.h"
#include "texture.h"
#include "vector.h"
#include "matrix.h"
#include "buffer.h"
#include "stage.h"

#include <list>


namespace kih
{	
	/* class RenderingContext
	*/
	RenderingContext::RenderingContext( size_t numRenderTargets ) :
		m_inputAssembler( std::make_unique<InputAssembler>( this ) ),
		m_vertexProcessor( std::make_unique<VertexProcessor>( this ) ),
		m_rasterizer( std::make_unique<Rasterizer>( this ) ),
		m_pixelProcessor( std::make_unique<PixelProcessor>( this ) ),
		m_outputMerger( std::make_unique<OutputMerger>( this ) ),
		m_depthWritable( false )
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
		byte depth = Float_ToByte( z );

		for ( int h = 0; h < height; ++h )
		{
			for ( int w = 0; w < width; ++w )
			{
				// FIXME: stencil
				ds->WriteTexel( w, h, depth, 0, 0 );
			}
		}
	}

	void RenderingContext::Draw( std::shared_ptr<IMesh> mesh )
	{
		if ( mesh == nullptr )
		{
			return;
		}

		RenderingDevice* pDevice = RenderingDevice::GetInstance();
		Assert( pDevice );
		Assert( m_inputAssembler );
		Assert( m_vertexProcessor );
		Assert( m_rasterizer );
		Assert( m_pixelProcessor );
		Assert( m_outputMerger );

		// Set a constant buffer for WVP transform.
		Matrix4 wv = pDevice->GetWorldMatrix() * pDevice->GetViewMatrix();
		Matrix4 wvp = wv * pDevice->GetProjectionMatrix();
		GetSharedConstantBuffer().SetMatrix4( ConstantBuffer::WVPMatrix, wvp );

		// Draw primitives here.
		PrimitiveType primitiveType = mesh->GetPrimitiveType();
		
		if ( primitiveType == PrimitiveType::Undefined )
		{
			if ( IrregularMesh* pMesh = dynamic_cast< IrregularMesh* >( mesh.get() ) )
			{
				for ( size_t face = 0; face < pMesh->NumFaces(); ++face )
				{
					const unsigned char* color = pMesh->GetFaceColor( face );
					GetSharedConstantBuffer().SetVector4( ConstantBuffer::DiffuseColor, 
															Vector4( static_cast<byte>( color[0] / 255.0f ), 
																	static_cast<byte>( color[1] / 255.0f ), 
																	static_cast<byte>( color[2] / 255.0f ), 
																	static_cast<byte>( color[3] / 255.0f ) ) );

					m_inputAssembler->SetFaceIndex( face );					
					DrawInternal( mesh, pMesh->NumVerticesInFace( face ) );
				}
			}
		}
		else
		{
			GetSharedConstantBuffer().SetVector4( ConstantBuffer::DiffuseColor, Vector4( 1.0f, 1.0f, 1.0f, 1.0f ) );

			DrawInternal( mesh, GetNumberOfVerticesPerPrimitive( primitiveType ) );
		}
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
		return false;
	}

	void RenderingContext::SetDepthWritable( bool writable )
	{
		m_depthWritable = writable;
	}

	void RenderingContext::SetDepthFunc( DepthFunc func )
	{
		// depth functions in lamda expression
		static DepthTestFunc DepthFunctions[] =
		{
			nullptr,											// None
			[]( byte src, byte dst ) { return src != dst; },	// Not
			[]( byte src, byte dst ) { return src == dst; },	// Equal
			[]( byte src, byte dst ) { return src < dst; },		// Less
			[]( byte src, byte dst ) { return src <= dst; },	// LessEqual
			[]( byte src, byte dst ) { return src > dst; },		// Greater
			[]( byte src, byte dst ) { return src >= dst; }		// GreaterEqual
		};

		static_assert( SIZEOF_ARRAY( DepthFunctions ) == static_cast< size_t >( DepthFunc::Size ), "incomplete depthfunc" );

		m_funcDepthTest = DepthFunctions[static_cast< int >( func )];
	}

	bool RenderingContext::CallDepthFunc( byte src, byte dst )
	{
		if ( m_funcDepthTest )
		{
			return m_funcDepthTest( src, dst );
		}

		// This is correct because no depth func means that a depth test is always passed.
		return true;
	}

	void RenderingContext::DrawInternal( std::shared_ptr<IMesh> mesh, int numVerticesPerPrimitive )
	{
		Assert( m_inputAssembler );
		Assert( m_vertexProcessor );
		Assert( m_rasterizer );
		Assert( m_pixelProcessor );
		Assert( m_outputMerger );

		// input assembler
		std::shared_ptr<VertexProcInputStream> vpInput = m_inputAssembler->Process( mesh );
		printf( "\nVertexProcInputStream Size: %d\n", vpInput->Size() );

		// vertex processor
		std::shared_ptr<RasterizerInputStream> raInput = m_vertexProcessor->Process( vpInput );
		printf( "RasterizerInputStream Size: %d\n", raInput->Size() );

		// rasterizer
		raInput->SetPrimitiveType( GetPrimitiveTypeFromNumberOfVertices( numVerticesPerPrimitive ) );
		std::shared_ptr<PixelProcInputStream> ppInput = m_rasterizer->Process( raInput );
		printf( "PixelProcInputStream Size: %d\n", ppInput->Size() );

		// pixel processor
		std::shared_ptr<OutputMergerInputStream> omInput = m_pixelProcessor->Process( ppInput );
		printf( "OutputMergerInputStream Size: %d\n", 0 );

		// output merger
		omInput = m_outputMerger->Process( omInput );
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
