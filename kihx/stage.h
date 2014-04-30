#pragma once

#include "base.h"
#include "buffer.h"
#include "mathlib.h"
#include "stream.h"



#define DEPTHTEST_FAST


namespace kih
{
	class VertexShaderInputStream;
	typedef VertexShaderInputStream InputAssemblerOutputStream;

	class RasterizerInputStream;
	typedef RasterizerInputStream VertexShaderOutputStream;

	class PixelShaderInputStream;
	typedef PixelShaderInputStream RasterizerOutputStream;

	class OutputMergerInputStream;
	typedef OutputMergerInputStream PixelShaderOutputStream;

	class IMesh;
	class Texture;
	class RenderingContext;


	/* class DepthBuffering
	*/
	class DepthBuffering final
	{
		NONCOPYABLE_CLASS( DepthBuffering );

	public:
		explicit DepthBuffering( RenderingContext* context );
		~DepthBuffering();

		FORCEINLINE bool IsValid() const
		{
			return ( m_ptr != nullptr );
		}

		FORCEINLINE byte* GetAddress( unsigned short x, unsigned short y )
		{
			return ( m_ptr + ( ( ( m_width * y ) + x ) * m_stride ) );
		}

		template<class T>
		FORCEINLINE T& GetValueRef( unsigned short x, unsigned short y )
		{
			return *( reinterpret_cast< T* >( m_ptr ) + ( ( m_width * y ) + x ) );
		}

		FORCEINLINE void SetWritable( bool writable )
		{
			m_depthWritable = writable;
		}

		FORCEINLINE bool Test( const PixelData& fragment )
		{
			VerifyReentry();

			float& dst = GetValueRef<float>( fragment.PX, fragment.PY );
			dst = !( fragment.Depth <= dst ) ? dst : fragment.Depth;
			return ( dst == fragment.Depth );
		}

		template<class T>
		bool Test( unsigned short x, unsigned short y, T depth )
		{
			Assert( IsValid() );
			Assert( x >= 0 && x < m_width );
			Assert( y >= 0 && y < m_ds->Height() );

			T& dst = GetValueRef<T>( x, y );

			switch ( m_depthFunc )
			{
			case DepthFunc::Not:
				if ( !( depth != dst ) )
				{
					return false;
				}
				break;

			case DepthFunc::Equal:
				if ( !( depth == dst ) )
				{
					return false;
				}
				break;

			case DepthFunc::Less:
				if ( !( depth < dst ) )
				{
					return false;
				}
				break;

			case DepthFunc::LessEqual:
				if ( !( depth <= dst ) )
				{
					return false;
				}
				break;

			case DepthFunc::Greater:
				if ( !( depth > dst ) )
				{
					return false;
				}
				break;

			case DepthFunc::GreaterEqual:
				if ( !( depth >= dst ) )
				{
					return false;
				}
				break;

			default:
				break;
			}

			// depth write
			if ( m_depthWritable )
			{
				VerifyReentry();

				dst = depth;
			}

			return true;
		}

	private:
		std::shared_ptr<Texture> m_ds;
		ColorFormat m_format;
		int m_width;
		int m_stride;
		byte* m_ptr;
		DepthFunc m_depthFunc;
		bool m_depthWritable;
	};


	/* class BaseGraphicsStage
	*/
	template<class InputStream, class OutputStream>
	class BaseGraphicsStage
	{
	public:
		explicit BaseGraphicsStage( RenderingContext* pContext ) :
			m_pRenderingContext( pContext )
		{
			Assert( m_pRenderingContext );

			m_outputStream = std::make_shared<OutputStream>();
		}

		virtual ~BaseGraphicsStage() = default;

		virtual std::shared_ptr<OutputStream> Process( const std::shared_ptr<InputStream>& inputStream ) = 0;

		FORCEINLINE const ConstantBuffer& GetSharedConstantBuffer()
		{
			return GetContext()->GetSharedConstantBuffer();
		}

		std::shared_ptr<UnorderedAccessView<InputStream>> GetUnorderedAccessView()
		{
			return m_uav;
		}

		FORCEINLINE void SetUnorderedAccessView( std::shared_ptr<UnorderedAccessView<InputStream>> uav )
		{
			m_uav = uav;
		}

	protected:
		FORCEINLINE RenderingContext* GetContext()
		{
			return m_pRenderingContext;
		}

	protected:
		std::shared_ptr<OutputStream> m_outputStream;
		std::shared_ptr<UnorderedAccessView<InputStream>> m_uav;

	private:
		RenderingContext* m_pRenderingContext;
	};


	/* class InputAssembler
		*/
	class InputAssembler : public BaseGraphicsStage<IMesh, InputAssemblerOutputStream>
	{
		NONCOPYABLE_CLASS( InputAssembler );

	public:
		explicit InputAssembler( RenderingContext* pRenderingContext ) :
			BaseGraphicsStage( pRenderingContext ),
			m_faceIndex( 0 )
		{
		}

		virtual ~InputAssembler()
		{
		}

		virtual std::shared_ptr<InputAssemblerOutputStream> Process( const std::shared_ptr<IMesh>& inputStream );

		FORCEINLINE void SetFaceIndex( size_t index )
		{
			m_faceIndex = index;
		}

		void BindFixedPipelineOutputStreamSource( std::shared_ptr<VertexShaderOutputStream> stream )
		{
			m_outputFixedPipeline = stream;
		}

	private:
		size_t m_faceIndex;
		std::shared_ptr<VertexShaderOutputStream> m_outputFixedPipeline;
	};


	/* class VertexShader
	*/
	class VertexShader : public BaseGraphicsStage<VertexShaderInputStream, VertexShaderOutputStream>
	{
		NONCOPYABLE_CLASS( VertexShader );

	public:
		explicit VertexShader( RenderingContext* pRenderingContext ) :
			BaseGraphicsStage( pRenderingContext )
		{
		}

		virtual ~VertexShader()
		{
		}
		
		virtual std::shared_ptr<VertexShaderOutputStream> Process( const std::shared_ptr<VertexShaderInputStream>& inputStream );

		std::shared_ptr<VertexShaderOutputStream> OutputStreamSource()
		{
			return m_outputStream;
		}

	private:
		void TransformWVP( const Vector3& position, const Matrix4& wvp, Vector4& outPosition ) const;
	};


	/* class Rasterizer
	*/
	class Rasterizer : public BaseGraphicsStage<RasterizerInputStream, RasterizerOutputStream>
	{
		NONCOPYABLE_CLASS( Rasterizer );

	public:
		explicit Rasterizer( RenderingContext* pRenderingContext ) :
			BaseGraphicsStage( pRenderingContext ),
			m_cullMode( CullMode::None )
		{
		}

		virtual ~Rasterizer()
		{
		}

		virtual std::shared_ptr<RasterizerOutputStream> Process( const std::shared_ptr<RasterizerInputStream>& inputStream );

		FORCEINLINE void SetCullMode( CullMode mode )
		{
			m_cullMode = mode;
		}

	private:
		struct EdgeTableElement
		{
			float YMax;
			float XMin;
			float XMax;
			float Slope;
			float ZStart;
			float ZEnd;

			EdgeTableElement() = delete;

			FORCEINLINE explicit EdgeTableElement( float yMax, float xMin, float xMax, float slope, float zStart, float zEnd ) :
				YMax( yMax ),
				XMin( xMin ),
				XMax( xMax ),
				Slope( slope ),
				ZStart( zStart ),
				ZEnd( zEnd )
			{
			}
		};

		struct ActiveEdgeTableElement
		{
			const EdgeTableElement* ET;
			float CurrentX;

			ActiveEdgeTableElement() = delete;

			FORCEINLINE explicit ActiveEdgeTableElement( const EdgeTableElement& etElem ) :
				ET( &etElem ),
				CurrentX( etElem.Slope > 0.0f ? etElem.XMin : etElem.XMax )
			{
			}

			// sort by x-less
			FORCEINLINE bool operator<( const ActiveEdgeTableElement& rhs )
			{
				return CurrentX < rhs.CurrentX;
			}
		};

		// scanline conversion
		void DoScanlineConversion( const std::shared_ptr<RasterizerInputStream>& inputStream, unsigned short width, unsigned short height );

		// rasterization using an edge table
		void RasterizeUsingEdgeTable( const std::shared_ptr<RasterizerInputStream>& inputStream, size_t numVerticesPerPrimitive, unsigned short width, unsigned short height );
		void GatherPixelsBeingDrawnFromScanlines( StlVector<ActiveEdgeTableElement>& aet, unsigned short minScanline, unsigned short maxScanline, unsigned short width, DepthBuffering& depthBufferingParam ) const;
		bool UpdateActiveEdgeTable( StlVector<ActiveEdgeTableElement>& aet, unsigned short scanline ) const;

		// rasterization using barycentric coordinates
		void RasterizeUsingBarycentricCoordinates( const std::shared_ptr<RasterizerInputStream>& inputStream, unsigned short width, unsigned short height );
		void RasterizeUsingBarycentricCoordinatesSSE( const std::shared_ptr<RasterizerInputStream>& inputStream, unsigned short width, unsigned short height );

		// transform
		void TransformViewport( const std::shared_ptr<RasterizerInputStream>& inputStream, unsigned short width, unsigned short height ) const;

	private:
		StlVector<StlVector<EdgeTableElement>> m_edgeTable;
		CullMode m_cullMode;
	};


	/* class PixelShader
	*/
	class PixelShader : public BaseGraphicsStage<PixelShaderInputStream, PixelShaderOutputStream>
	{
		NONCOPYABLE_CLASS( PixelShader );
		
	public:
		explicit PixelShader( RenderingContext* pRenderingContext ) :
			BaseGraphicsStage( pRenderingContext )
		{
		}

		virtual ~PixelShader()
		{
		}

		virtual std::shared_ptr<PixelShaderOutputStream> Process( const std::shared_ptr<PixelShaderInputStream>& inputStream );
	};


	/* class OutputMerger
	*/
	class OutputMerger : public BaseGraphicsStage<OutputMergerInputStream, OutputMergerOutputStream>
	{
		NONCOPYABLE_CLASS( OutputMerger );

	public:
		explicit OutputMerger( RenderingContext* pRenderingContext ) :
			BaseGraphicsStage( pRenderingContext )
		{
		}

		virtual ~OutputMerger()
		{
		}

		virtual std::shared_ptr<OutputMergerOutputStream> Process( const std::shared_ptr<OutputMergerInputStream>& inputStream );

		// Do traditional depth buffering and pixel operations.
		void Resolve( const std::shared_ptr<OutputMergerInputStream>& inputStream );
	};
};
