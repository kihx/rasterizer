#pragma once

#include "base.h"
#include "stream.h"
#include "buffer.h"

#include <vector>
#include <list>


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

		FORCEINLINE bool Execute( unsigned short x, unsigned short y, float depth )
		{
			if ( m_format == ColorFormat::D8S24 )
			{
				return ExecuteInternal( x, y, Float_ToByte( depth ) );
			}
			else
			{
				return ExecuteInternal( x, y, depth );
			}
		}

	private:
		template<class T>
		bool ExecuteInternal( unsigned short x, unsigned short y, T depth );

	private:
		ColorFormat m_format;
		DepthFunc m_depthFunc;
		int m_width;
		int m_stride;
		byte* m_ptr;
		std::shared_ptr<Texture> m_ds;
#ifdef DEPTHFUNC_LAMDA
		RenderingContext* m_context;
#endif
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

	private:
		size_t m_faceIndex;
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
			BaseGraphicsStage( pRenderingContext )
		{
		}

		virtual ~Rasterizer()
		{
		}

		virtual std::shared_ptr<RasterizerOutputStream> Process( const std::shared_ptr<RasterizerInputStream>& inputStream );

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

			explicit EdgeTableElement( float yMax, float xMin, float xMax, float slope, float zStart, float zEnd ) :
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

			explicit ActiveEdgeTableElement( const EdgeTableElement& etElem ) :
				ET( &etElem ),
				CurrentX( etElem.Slope > 0.0f ? etElem.XMin : etElem.XMax )
			{
			}

			// sort by x-less
			bool operator<( const ActiveEdgeTableElement& rhs )
			{
				return CurrentX < rhs.CurrentX;
			}
		};

		// scanline conversion
		void DoScanlineConversion( const std::shared_ptr<RasterizerInputStream>& inputStream, std::shared_ptr<RasterizerOutputStream> outputStream, unsigned short width, unsigned short height );
		void GatherPixelsBeingDrawnFromScanlines( std::shared_ptr<RasterizerOutputStream> outputStream, std::vector<ActiveEdgeTableElement>& aet, unsigned short minScanline, unsigned short maxScanline, unsigned short width, DepthBuffering& depthBufferingParam );
		bool UpdateActiveEdgeTable( std::vector<ActiveEdgeTableElement>& aet, unsigned short scanline ) const;

		// transform
		void TransformViewport( const std::shared_ptr<RasterizerInputStream>& inputStream, unsigned short width, unsigned short height ) const;

	private:
		std::vector<std::vector<EdgeTableElement>> m_edgeTable;
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
