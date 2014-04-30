#pragma once

#include "base.h"
#include "buffer.h"
#include "matrix.h"
#include "stdsupport.h"

#include <vector>


namespace kih
{	
	class IMesh;
	class IrregularMesh;
	class OptimizedMesh;

	class Texture;

	class VertexShaderInputStream;	
	typedef VertexShaderInputStream InputAssemblerOutputStream;

	class RasterizerInputStream;
	typedef RasterizerInputStream VertexShaderOutputStream;

	class PixelShaderInputStream;
	typedef PixelShaderInputStream RasterizerOutputStream;

	class OutputMergerInputStream;
	typedef OutputMergerInputStream PixelShaderOutputStream;
	
	class OutputMergerOutputStream;

	template<class T>
	class UnorderedAccessView;

	class InputAssembler;
	class VertexShader;
	class Rasterizer;
	class PixelShader;
	class OutputMerger;
	class RenderingContext;
	

	/* struct Viewport
	*/
	struct Viewport
	{
		unsigned short X;
		unsigned short Y;
		unsigned short Width;
		unsigned short Height;
	};


	/* class RenderingContext
	*/
	class RenderingContext final
	{
		NONCOPYABLE_CLASS( RenderingContext );
		
		using DepthTestFunc = std::function< bool( byte /*src*/, byte /*dst*/ ) >;

	public:
		explicit RenderingContext( size_t numRenderTargets );

		// fixed pipeline
		FORCEINLINE bool IsFixedPipelineMode() const
		{
			return m_fixedPipelineMode;
		}

		FORCEINLINE void SetFixedPipelineMode( bool mode )
		{
			m_fixedPipelineMode = mode;
		}

		// draw
		void Clear( byte r, byte g, byte b, byte a, float z = 1.0f, int stencil = 0 );		
		void Draw( const std::shared_ptr<IMesh>& mesh );
		
		// In FuncPreRender, the world matrix of the specified mesh should be set onto the constant buffer of the rendering context.
		using FuncPreRender = std::function<void( std::shared_ptr<RenderingContext> context, size_t index )>;
		static void DrawInParallel( StlVector<std::shared_ptr<RenderingContext>>& contexts, const StlVector<std::shared_ptr<IMesh>>& meshes, int meshCount, FuncPreRender funcPreRender );

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

		void SetUnorderedAccessView( std::shared_ptr<UnorderedAccessView<OutputMergerInputStream>> omUAV );

		// constant buffers
		// SharedConstantBuffer can be accessed in all rendering stages. 
		FORCEINLINE ConstantBuffer& GetSharedConstantBuffer()
		{
			return m_sharedConstantBuffer;
		}

		// render states
		FORCEINLINE const Viewport& GetViewport() const
		{
			return m_viewport;
		}

		void SetViewport( unsigned short x, unsigned short y, unsigned short width, unsigned short height )
		{
			m_viewport.X = x;
			m_viewport.Y = y;
			m_viewport.Width = width;
			m_viewport.Height = height;
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

		FORCEINLINE CullMode GetCullMode() const
		{
			return m_cullMode;
		}
		void SetCullMode( CullMode mode )
		{
			m_cullMode = mode;
		}

	private:
		void DrawInternal( const std::shared_ptr<IMesh>& mesh, PrimitiveType primitiveType = PrimitiveType::Triangles );

		// Resolve the UAV to display. This function processes the traditional output merger stage using the UAC.
		// Note that UAV and RT must be binded.
		void ResolveUnorderedAccessViews( StlVector<std::shared_ptr<RenderingContext>>& contexts );

	private:
		// render stages
		std::shared_ptr<InputAssembler> m_inputAssembler;
		std::shared_ptr<VertexShader> m_vertexShader;
		std::shared_ptr<Rasterizer> m_rasterizer;
		std::shared_ptr<PixelShader> m_pixelShader;
		std::shared_ptr<OutputMerger> m_outputMerger;
		
		// render target
		StlVector< std::shared_ptr<Texture> > m_renderTargets;
		std::shared_ptr<Texture> m_depthStencil;

		// constant buffer
		ConstantBuffer m_sharedConstantBuffer;

		// render states
		Viewport m_viewport;

		// depth buffering
		DepthFunc m_depthFunc;
		bool m_depthWritable;

		CullMode m_cullMode;

		bool m_fixedPipelineMode;
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
		StlVector<std::shared_ptr<RenderingContext>> m_renderingContexts;
		Matrix4 m_worldMatrix;
		Matrix4 m_viewMatrix;
		Matrix4 m_projMatrix;
	};
};

using kih::DepthFunc;

using kih::RenderingContext;
using kih::RenderingDevice;
