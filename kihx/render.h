#pragma once

#include "base.h"
#include "matrix.h"
#include "buffer.h"

#include <vector>
#include <functional>


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
	typedef OutputMergerInputStream OutputMergerOutputStream;

	class InputAssembler;
	class VertexProcessor;
	class Rasterizer;
	class PixelProcessor;
	class OutputMerger;
	class RenderingContext;

	
	/* enum class DepthFunc
	*/
	enum class DepthFunc
	{
		None = 0,
		Not,
		Equal,
		Less,
		LessEqual,
		Greater,
		GreaterEqual,
		/* the number of enum elements */
		Size
	};
	

	/* class RenderingContext
	*/
	class RenderingContext final
	{
		NONCOPYABLE_CLASS( RenderingContext );
		
		using DepthTestFunc = std::function< bool( byte /*src*/, byte /*dst*/ ) >;

	public:
		explicit RenderingContext( size_t numRenderTargets );

		// draw
		void Clear( byte r, byte g, byte b, byte a, float z = 1.0f, int stencil = 0 );
		void Draw( std::shared_ptr<IMesh> mesh );

		// render targets
		std::shared_ptr<Texture> GetRenderTaget( size_t index )
		{
			Assert( ( index >= 0 && index < m_renderTargets.size() ) && "out of ranged index" );
			return m_renderTargets[index];
		}

		std::shared_ptr<Texture> GetDepthStencil()
		{
			return m_depthStencil;
		}

		bool SetRenderTarget( std::shared_ptr<Texture> texture, size_t index );
		bool SetDepthStencil( std::shared_ptr<Texture> texture );

		// constant buffers
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

		void SetDepthFunc( DepthFunc func );
		bool CallDepthFunc( byte src, byte dst );

	private:
		void DrawInternal( std::shared_ptr<IMesh> mesh, int numVerticesPerPrimitive );

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
		DepthTestFunc m_funcDepthTest;
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
