#include "stdafx.h"
#include "render.h"
#include "mesh.h"
#include "texture.h"
#include "vector.h"
#include "matrix.h"
#include "buffer.h"
#include "stage.h"
#include "concommand.h"
#include "threading.h"
#include "profiler.h"

#include <list>
#include <map>
#include <thread>


#define USE_THREAD_POOL
#define PREEMPTIVE_PARALLEL

#define Thread std::thread


namespace kih
{	
	/* class RenderingContext
	*/
	RenderingContext::RenderingContext( size_t numRenderTargets ) :
		m_inputAssembler( std::make_shared<InputAssembler>( this ) ),
		m_vertexProcessor( std::make_shared<VertexProcessor>( this ) ),
		m_rasterizer( std::make_shared<Rasterizer>( this ) ),
		m_pixelProcessor( std::make_shared<PixelProcessor>( this ) ),
		m_outputMerger( std::make_shared<OutputMerger>( this ) ),
		m_depthFunc( DepthFunc::None ),
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
		int bytePerPixel = GetBytesPerPixel( ds->Format() );

		switch ( ds->Format() )
		{
		case ColorFormat::D8S24:
			{
				// FIXME: stencil value
				memset( ptr, Float_ToByte( z ), width * height * bytePerPixel );
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
		Assert( m_vertexProcessor );
		Assert( m_rasterizer );
		Assert( m_pixelProcessor );
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
					DrawInternal( mesh, GetPrimitiveTypeFromNumberOfVertices( pMesh->NumVerticesInFace( face ) ) );
				}
			}
		}
		else
		{
			DrawInternal( mesh );
		}
	}

	void RenderingContext::DrawInParallel( const std::vector<std::shared_ptr<RenderingContext>>& contexts, const std::vector<std::shared_ptr<IMesh>>& meshes, FuncPreRender funcPreRender )
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

		// If the number of meshes is less than 4, 
		// draw all meshes on the first context without parallel processing.
		if ( meshes.size() < 4 )
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
		
		// Note that the world matrix would be multiplied just before drawing a mesh
		// because the world matrix of a mesh is different each other.
		// So premultiply view-projection matrices only.
		Matrix4 viewProj = pDevice->GetViewMatrix() * pDevice->GetProjectionMatrix();

		size_t numContexts = contexts.size();
#ifdef PREEMPTIVE_PARALLEL
		// nothing
#else
		// Distribute input meshes to each context.
		size_t numMeshes = meshes.size();
		size_t numMeshesPerContext = static_cast<size_t>( std::ceil( numMeshes / static_cast<float>( numContexts ) ) );
#endif

		// Reserve the series of the input stream of the output merge
		// which are the result of the work for each rendering context.
		// We should sequentially merge them on the output merger in a single thread (the first context).
		std::vector<std::shared_ptr<OutputMergerInputStream>> omInputStreamList;
		omInputStreamList.reserve( numContexts );
		
		// a mutext to collect thread-safely
		static Mutex OutputCollectorMutex;
		
#ifdef USE_THREAD_POOL
		ThreadPool* threadPool = ThreadPool::GetInstance();
#else
		// thread container to join after working
		std::vector<Thread> threads;
		threads.resize( numContexts );
#endif

#ifdef PREEMPTIVE_PARALLEL
		Atomic<int> workIndex( 0 );
#endif
		for ( size_t c = 0; c < numContexts; ++c )
		{
			auto context = contexts[c];
			Assert( context );

#ifdef PREEMPTIVE_PARALLEL
			auto task = std::bind(
				[=, &workIndex, &omInputStreamList]( std::shared_ptr<RenderingContext> context, Matrix4 viewProj, FuncPreRender funcPreRender )
				{
					bool hasRT = ( context->GetRenderTagetConst( 0 ) != nullptr );

					ConstantBuffer& cbuffer = context->GetSharedConstantBuffer();

					std::vector<std::shared_ptr<OutputMergerInputStream>> output;
					output.reserve( meshes.size() );					

					int meshCount = static_cast< int >( meshes.size() );

					// Until the last index...
					while ( workIndex.Value() < meshCount )
					{
						// Prefech the workIndex to avoid changing the value on other thread, and then increment the workIndex.
						int index = workIndex.FetchAndIncrement();

						// Call the PreRender functor.
						funcPreRender( context, index );

						// Now compute the final WVP matrix of this mesh.
						Matrix4 wvp = cbuffer.GetMatrix4( ConstantBuffer::WorldMatrix ) * viewProj;
						cbuffer.SetMatrix4( ConstantBuffer::WVPMatrix, wvp );

						// Run the rendering pipeline for the mesh.
						std::shared_ptr<OutputMergerInputStream> omInput = context->RunRenderingPipeline( meshes[index] );

						// If a context has a RT, go to the output merger stage.
						if ( hasRT )
						{
							context->m_outputMerger->Process( omInput );
						}
						else
						{
							// Otherwise, collect the input stream for late output merge.
							output.emplace_back( std::move( omInput->Clone() ) );
						}
					}

					// Collect all of output stream.
					if ( !hasRT )
					{
						LockGuard<Mutex> lockGuard( OutputCollectorMutex );
						std::copy( output.begin(), output.end(), std::back_inserter( omInputStreamList ) );
					}
				},
				context,
				viewProj,
				funcPreRender );

#ifdef USE_THREAD_POOL
			threadPool->Queue( task );
#else
 			Thread t { task };
#endif

#else // PREEMPTIVE_PARALLEL
			// Distribute input meshes to each context.
			std::map<size_t, std::shared_ptr<IMesh>> inputMeshes;			
			for ( size_t m = 0; m < numMeshesPerContext; ++m )
			{
				size_t index = c * numMeshesPerContext + m;
				if ( index < 0 || index >= numMeshes )
				{
					continue;
				}

				const auto& mesh = meshes[index];
				if ( mesh == nullptr )
				{
					continue;
				}
				
				if ( mesh->GetPrimitiveType() != PrimitiveType::Triangles )
				{
					continue;
				}

				inputMeshes.emplace( index, mesh );
			}

			Thread t
			{
				[&omInputStreamList]( std::shared_ptr<RenderingContext> context, std::map<size_t, std::shared_ptr<IMesh>> inputMeshes, Matrix4 viewProj, FuncPreRender funcPreRender )
				{
					std::vector<std::shared_ptr<OutputMergerInputStream>> output;
					output.reserve( inputMeshes.size() );

					ConstantBuffer& cbuffer = context->GetSharedConstantBuffer();

					for ( auto iter = inputMeshes.begin(); iter != inputMeshes.end(); ++iter )
					{										
						// Call the PreRender functor.
						funcPreRender( context, iter->first );

						// Now compute the final WVP matrix of this mesh.
						Matrix4 wvp = cbuffer.GetMatrix4( ConstantBuffer::WorldMatrix ) * viewProj;
						cbuffer.SetMatrix4( ConstantBuffer::WVPMatrix, wvp );

						// Run the rendering pipeline for the mesh.
						std::shared_ptr<OutputMergerInputStream> omInput = context->RunRenderingPipeline( iter->second );

						// And then collect the input stream for the output merger.
						output.emplace_back( omInput->Clone() );
					}

					// Collect all of output stream.
					LockGuard<Mutex> lockGuard( OutputCollectorMutex );
					std::copy( output.begin(), output.end(), std::back_inserter( omInputStreamList ) );
				},
				context,
				inputMeshes,
				viewProj,
				funcPreRender
			};		
#endif	// PREEMPTIVE_PARALLEL

#ifdef USE_THREAD_POOL
			// do nothing
#else
			t.swap( threads[c] );
#endif
		}

		// Join all rendering threads.
#ifdef USE_THREAD_POOL
		threadPool->WaitForAllTasks();
#else
		for ( size_t i = 0; i < threads.size(); ++i )
		{
			threads[i].join();
		}
#endif

		// Merge all input stream data of the output merger on the first rendering context.
		// We assume that rendering order is independent.
		auto context = contexts[0];
		for ( auto stream : omInputStreamList )
		{
			context->m_outputMerger->Process( stream );
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
		return true;
	}

	void RenderingContext::SetDepthWritable( bool writable )
	{
		m_depthWritable = writable;
	}

	void RenderingContext::SetDepthFunc( DepthFunc func )
	{
#ifdef DEPTHFUNC_LAMDA
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
#endif
		m_depthFunc = func;
	}

#ifdef DEPTHFUNC_LAMDA
	bool RenderingContext::CallDepthFunc( byte src, byte dst )
	{
		if ( m_funcDepthTest )
		{
			return m_funcDepthTest( src, dst );
		}

		// This is correct because no depth func means that a depth test is always passed.
		return true;
	}
#endif

	void RenderingContext::DrawInternal( const std::shared_ptr<IMesh>& mesh, PrimitiveType primitiveType /*= PrimitiveType::Triangles */ )
	{
		Assert( m_outputMerger );

		// Run the rendering pipeline from the input assembler to the output merger at a time.
		m_outputMerger->Process( RunRenderingPipeline( mesh, primitiveType ) );
		// Note that the output stream of the output merger is meaningless
		// because the output merger directly write data on render targets and a depth-stencil buffer.
	}

	std::shared_ptr<OutputMergerInputStream> RenderingContext::RunRenderingPipeline( const std::shared_ptr<IMesh>& mesh, PrimitiveType primitiveType )
	{
		Assert( m_inputAssembler );
		Assert( m_vertexProcessor );
		Assert( m_rasterizer );
		Assert( m_pixelProcessor );

		// input assembler stage
		std::shared_ptr<VertexProcInputStream> vpInput = m_inputAssembler->Process( mesh );
		//printf( "\nVertexProcInputStream Size: %d\n", vpInput->Size() );
	
		// vertex processor stage
		std::shared_ptr<RasterizerInputStream> raInput = m_vertexProcessor->Process( vpInput );
		//printf( "RasterizerInputStream Size: %d\n", raInput->Size() );

		// rasterizer stage
		raInput->SetPrimitiveType( primitiveType );
		std::shared_ptr<PixelProcInputStream> ppInput = m_rasterizer->Process( raInput );
		//printf( "PixelProcInputStream Size: %d\n", ppInput->Size() );

		// pixel processor stage
		std::shared_ptr<OutputMergerInputStream> omInput = m_pixelProcessor->Process( ppInput );
		//printf( "OutputMergerInputStream Size: %d\n", 0 );

		return omInput;
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
