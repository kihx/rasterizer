#pragma once

#include "base.h"
#include "stream.h"
#include "buffer.h"

#include <vector>
#include <list>


namespace kih
{
	class VertexProcInputStream;
	typedef VertexProcInputStream InputAssemblerOutputStream;

	class RasterizerInputStream;
	typedef RasterizerInputStream VertexProcOutputStream;

	class PixelProcInputStream;
	typedef PixelProcInputStream RasterizerOutputStream;

	class OutputMergerInputStream;
	typedef OutputMergerInputStream PixelProcOutputStream;

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
		template<typename T>
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

	protected:
		FORCEINLINE RenderingContext* GetContext()
		{
			return m_pRenderingContext;
		}

	protected:
		std::shared_ptr<OutputStream> m_outputStream;

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


	/* class VertexProcessor
	*/
	class VertexProcessor : public BaseGraphicsStage<VertexProcInputStream, VertexProcOutputStream>
	{
		NONCOPYABLE_CLASS( VertexProcessor );

	public:
		explicit VertexProcessor( RenderingContext* pRenderingContext ) :
			BaseGraphicsStage( pRenderingContext )
		{
		}

		virtual ~VertexProcessor()
		{
		}

		virtual std::shared_ptr<VertexProcOutputStream> Process( const std::shared_ptr<VertexProcInputStream>& inputStream );

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

			//const Color32& ColorL;
			//const Color32& ColorR;

			EdgeTableElement() = delete;

			explicit EdgeTableElement( float yMax, float xMin, float xMax, float slope, float zStart, float zEnd /*, const Color32& colorL, const Color32& colorR*/ ) :
				YMax( yMax ),
				XMin( xMin ),
				XMax( xMax ),
				Slope( slope ),
				ZStart( zStart ),
				ZEnd( zEnd )/*,
				ColorL( colorL ),
				ColorR( colorR )*/
			{
			}
		};

		struct ActiveEdgeTableElement
		{
			NONCOPYABLE_STRUCT( ActiveEdgeTableElement );

			const EdgeTableElement& ET;
			float CurrentX;

			ActiveEdgeTableElement() = delete;

			explicit ActiveEdgeTableElement( const EdgeTableElement& etElem ) :
				ET( etElem ),
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
		void GatherPixelsBeingDrawnFromScanlines( std::shared_ptr<RasterizerOutputStream> outputStream, unsigned short minScanline, unsigned short maxScanline, unsigned short width, DepthBuffering& depthBufferingParam );
		bool UpdateActiveEdgeTable( std::list<ActiveEdgeTableElement>& aet, unsigned short scanline ) const;

		// transform
		void TransformViewport( const std::shared_ptr<RasterizerInputStream>& inputStream, unsigned short width, unsigned short height ) const;

	private:
		std::vector<std::vector<EdgeTableElement>> m_edgeTable;
	};


	/* class PixelProcessor
	*/
	class PixelProcessor : public BaseGraphicsStage<PixelProcInputStream, PixelProcOutputStream>
	{
		NONCOPYABLE_CLASS( PixelProcessor );
		
	public:
		explicit PixelProcessor( RenderingContext* pRenderingContext ) :
			BaseGraphicsStage( pRenderingContext )
		{
		}

		virtual ~PixelProcessor()
		{
		}

		virtual std::shared_ptr<PixelProcOutputStream> Process( const std::shared_ptr<PixelProcInputStream>& inputStream );
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
	};
};
