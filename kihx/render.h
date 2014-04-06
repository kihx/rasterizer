#pragma once

#include "base.h"
#include "matrix.h"
#include "buffer.h"

#include <vector>


//#define DEPTHFUNC_LAMDA


namespace kih
{	
	class IMesh;
	class IrregularMesh;
	class OptimizedMesh;

	class Texture;

	class VertexProcInputStream;	
	typedef VertexProcInputStream InputAssemblerOutputStream;

	class RasterizerInputStream;
	typedef RasterizerInputStream VertexProcOutputStream;

	class PixelProcInputStream;
	typedef PixelProcInputStream RasterizerOutputStream;

	class OutputMergerInputStream;
	typedef OutputMergerInputStream PixelProcOutputStream;
	
	class OutputMergerOutputStream;

	class InputAssembler;
	class VertexProcessor;
	class Rasterizer;
	class PixelProcessor;
	class OutputMerger;
	class RenderingContext;
	

	/* class RenderingContext
	*/
	class RenderingContext final
	{
		NONCOPYABLE_CLASS( RenderingContext );
		
		using DepthTestFunc = std::function< bool( byte /*src*/, byte /*dst*/ ) >;

	public:
		static const int ThreadConcurrency = 4;

		explicit RenderingContext( size_t numRenderTargets );

		// draw
		void Clear( byte r, byte g, byte b, byte a, float z = 1.0f, int stencil = 0 );		
		void Draw( const std::shared_ptr<IMesh>& mesh );
		
		// In FuncPreRender, the world matrix of the specified mesh should be set onto the constant buffer of the rendering context.
		using FuncPreRender = std::function<void( std::shared_ptr<RenderingContext> context, const std::shared_ptr<IMesh>& mesh, size_t index )>;
		static void DrawInParallel( std::vector<std::shared_ptr<RenderingContext>>& contexts, const std::vector<std::shared_ptr<IMesh>>& meshes, FuncPreRender funcPreRender );

		// render targets
		FORCEINLINE size_t NumberOfRenderTargets() const
		{
			return m_renderTargets.size();
		}

		const std::shared_ptr<Texture>& GetRenderTagetConst( size_t index ) const
		{
			Assert( index >= 0 && index < m_renderTargets.size() );
			return m_renderTargets[index];
		}

		std::shared_ptr<Texture> GetRenderTaget( size_t index )
		{
			Assert( index >= 0 && index < m_renderTargets.size() );
			return m_renderTargets[index];
		}

		std::shared_ptr<Texture> GetDepthStencil()
		{
			return m_depthStencil;
		}

		bool SetRenderTarget( std::shared_ptr<Texture> texture, size_t index );
		bool SetDepthStencil( std::shared_ptr<Texture> texture );

		// constant buffers
		// SharedConstantBuffer can be accessed in all rendering stages. 
		FORCEINLINE ConstantBuffer& GetSharedConstantBuffer()
		{
			return m_sharedConstantBuffer;
		}

		// depth buffering
		FORCEINLINE bool DepthWritable() const 
		{
			return m_depthWritable;
		}
		void SetDepthWritable( bool writable );

		FORCEINLINE DepthFunc DepthFunction() const
		{
			return m_depthFunc;
		}
		void SetDepthFunc( DepthFunc func );
#ifdef DEPTHFUNC_LAMDA
		bool CallDepthFunc( byte src, byte dst );
#endif

	private:
		void DrawInternal( const std::shared_ptr<IMesh>& mesh, PrimitiveType primitiveType = PrimitiveType::Triangles );

		// Run the rendering pipeline from the input assembler stage to the pixel processor stage,
		// and return the input stream for the output merger.
		// To display the rendering result, we must run the output merger stage using the input stream.
		std::shared_ptr<OutputMergerInputStream> RunRenderingPipeline( const std::shared_ptr<IMesh>& mesh, PrimitiveType primitiveType = PrimitiveType::Triangles );		

	private:
		// render stages
		std::unique_ptr<InputAssembler> m_inputAssembler;
		std::unique_ptr<VertexProcessor> m_vertexProcessor;
		std::unique_ptr<Rasterizer> m_rasterizer;
		std::unique_ptr<PixelProcessor> m_pixelProcessor;
		std::unique_ptr<OutputMerger> m_outputMerger;
		
		// render target
		std::vector< std::shared_ptr<Texture> > m_renderTargets;
		std::shared_ptr<Texture> m_depthStencil;

		// constant buffer
		ConstantBuffer m_sharedConstantBuffer;

		// depth buffering
#ifdef DEPTHFUNC_LAMDA
		DepthTestFunc m_funcDepthTest;
#endif
		DepthFunc m_depthFunc;
		bool m_depthWritable;
	};


	/* class RenderingDevice
	*/
	class RenderingDevice final : public Singleton<RenderingDevice>
	{
		friend class Singleton<RenderingDevice>;

	private:
		RenderingDevice()
		{
		};

	public:
		~RenderingDevice()
		{
		};

		// factory
		std::shared_ptr<RenderingContext> CreateRenderingContext();

		FORCEINLINE size_t NumberOfRenderingContexts()
		{
			return m_renderingContexts.size();
		}

		std::shared_ptr<RenderingContext> GetRenderingContext( size_t index )
		{
			Assert( index >= 0 && index < m_renderingContexts.size() );
			return m_renderingContexts[index];
		}

		// WVP matrices
		FORCEINLINE const Matrix4& GetWorldMatrix() const
		{
			return m_worldMatrix;
		}

		FORCEINLINE const Matrix4& GetViewMatrix() const
		{
			return m_viewMatrix;
		}

		FORCEINLINE const Matrix4& GetProjectionMatrix() const
		{
			return m_projMatrix;
		}

		void SetWorldMatrix( const Matrix4& m );
		void SetViewMatrix( const Matrix4& m );
		void SetProjectionMatrix( const Matrix4& m );

	private:
		std::vector<std::shared_ptr<RenderingContext>> m_renderingContexts;
		Matrix4 m_worldMatrix;
		Matrix4 m_viewMatrix;
		Matrix4 m_projMatrix;
	};
};

using kih::DepthFunc;

using kih::RenderingContext;
using kih::RenderingDevice;
