#include "stdafx.h"
#include "render.h"
#include "mesh.h"

#include <list>


namespace kih
{
	/* class Texture
	*/
	std::shared_ptr<Texture> Texture::Create( int width, int height, ColorFormat format, void* pExternalMemory )
	{
		// cannot use std::make_shared<>() by the protected access modifier
		auto texture = std::shared_ptr<Texture>( new Texture() );
		texture->m_width = width;
		texture->m_height = height;
		texture->m_format = format;
		texture->m_flags = 0;

		if ( pExternalMemory )
		{
			texture->SetExternalMemory( pExternalMemory );
		}
		else
		{
			texture->m_pMemory = malloc( width * height * ComputeBytesPerPixel( format ) );
		}

		return texture;
	}


	struct VertexProcData
	{
		// object-space position
		union
		{
			struct
			{
				float X;
				float Y;
				float Z;
			};

			float Position[3];
		};

		// vertex color
		Color32 Color;

		VertexProcData() :
			X( 0.0f ),
			Y( 0.0f ),
			Z( 0.0f )			
		{
		}

		VertexProcData( const float position[3], const Color32& color) :
			X( position[0] ),
			Y( position[1] ),
			Z( position[2] ),
			Color( color )
		{
		}

		VertexProcData( const VertexProcData& data ) = default;
	};

	/* class VertexProcInputStream
	*/
	class VertexProcInputStream : public BaseInputOutputStream<VertexProcData>
	{
	public:
		VertexProcInputStream()
		{
		}
	};


	struct RasterizerData
	{
		// in NDC coordinates
		union
		{
			struct
			{
				float X;
				float Y;
				float Z;
				float W;
			};

			float Position[4];
		};

		// vertex color
		Color32 Color;

		RasterizerData() :
			X( 0.0f ),
			Y( 0.0f ),
			Z( 0.0f ),
			W( 1.0f )			
		{
		}

		RasterizerData( const float position[4], const Color32& color ) :
			X( position[0] ),
			Y( position[1] ),
			Z( position[2] ),
			W( position[3] ),
			Color( color )
		{
		}

		RasterizerData( const RasterizerData& data ) = default;
	};

	/* class RasterizerInputStream
	*/
	class RasterizerInputStream : public BaseInputOutputStream<RasterizerData>
	{
	public:
		RasterizerInputStream()
		{
		}
	};


	struct PixelProcData
	{
		// x and y coordinates of a pixel
		unsigned short PX;
		unsigned short PY;
		// z of a pixel
		float PZ;

		// interpolated color
		Color32 Color;

		PixelProcData() :
			PX( 0 ),
			PY( 0 ),
			PZ( 1.0f ) // farthest
		{
		}

		PixelProcData( unsigned short px, unsigned short py, float pz, const Color32& color ) :
			PX( px ),
			PY( py ),
			PZ( pz ),
			Color( color )
		{
		}

		PixelProcData( const PixelProcData& data ) = default;
	};

	/* class PixelProcInputStream
	*/
	class PixelProcInputStream : public BaseInputOutputStream<PixelProcData>
	{
	public:
		PixelProcInputStream()
		{
		}
	};


	/* class OutputMergerInputStream
	*/
	__UNDONE( Need output merger stream );
	class OutputMergerInputStream
	{
	public:
		OutputMergerInputStream()
		{
		}

	private:

	};



	/* class InputAssembler
	*/
	class InputAssembler : public IRenderingProcessor<Mesh, InputAssemblerOutputStream>
	{
		NONCOPYABLE_CLASS( InputAssembler )

	public:
		explicit InputAssembler( RenderingContext* pRenderingContext ) :
			m_renderingContext( pRenderingContext )
		{
		}

		virtual ~InputAssembler()
		{
		}

		virtual std::shared_ptr<InputAssemblerOutputStream> Process( std::shared_ptr<Mesh> inputStream )
		{
			auto outputStream = std::make_shared<InputAssemblerOutputStream>();

			// Count the number of vertices in the mesh
#ifdef SUPPORT_MSH
			size_t capacity = 0;
			size_t numFaces = inputStream->NumFaces();
			for ( size_t face = 0; face < numFaces; ++face )
			{
				capacity += inputStream->NumVerticesInFace( face );
			}
			outputStream->Reserve( capacity );

			// Convert the mesh to an input stream of the vertex processor
			for ( size_t face = 0; face < numFaces; ++face )
			{
				const byte* color = inputStream->GetFaceColor( face );

				size_t numVertices = inputStream->NumVerticesInFace( face );
				for ( size_t vert = 0; vert < numVertices; ++vert )
				{
					__TODO( change vertex color to material color using a pixel processor constant buffer );
					outputStream->Push( inputStream->GetVertexInFaceAt( face, vert ), color );
				}
			}
#else
			static_assert( 0, "not implemented yet" );
#endif

			return outputStream;
		}

	private:
		std::shared_ptr<RenderingContext> m_renderingContext;
	};
	

	/* class VertexProcessor
	*/
	class VertexProcessor : public IRenderingProcessor<VertexProcInputStream, VertexProcOutputStream>
	{
		NONCOPYABLE_CLASS( VertexProcessor )

	public:
		explicit VertexProcessor( RenderingContext* pRenderingContext ) :
			m_renderingContext( pRenderingContext )
		{
		}

		virtual ~VertexProcessor()
		{
		}

		virtual std::shared_ptr<VertexProcOutputStream> Process( std::shared_ptr<VertexProcInputStream> inputStream )
		{
			auto outputStream = std::make_shared<VertexProcOutputStream>();

			size_t inputSize = inputStream->Size();
			for ( size_t i = 0; i < inputSize; ++i )
			{
				const auto& vertex = inputStream->GetData( i );
				
				float transformedPosition[4];
				Transform( vertex.Position, transformedPosition );

				outputStream->Push( transformedPosition, vertex.Color );
			}

			return outputStream;
		}

	private:
		void Transform( const float inPosition[3], float outPosition[4] )
		{
			// TODO: transform
			outPosition[0] = inPosition[0];
			outPosition[1] = inPosition[1];
			outPosition[2] = inPosition[2];
			outPosition[3] = 1.0f;
		}

		//IOutputStream* Process( IInputStream* inputStream )
		//{
		//	return nullptr;
		//}

	private:
		std::shared_ptr<RenderingContext> m_renderingContext;
	};

	
	/* class Rasterizer
	*/
	class Rasterizer : public IRenderingProcessor<RasterizerInputStream, RasterizerOutputStream>
	{
		NONCOPYABLE_CLASS( Rasterizer )

	public:
		explicit Rasterizer( RenderingContext* pRenderingContext ) :
			m_renderingContext( pRenderingContext )
		{
		}

		virtual ~Rasterizer()
		{
		}

		virtual std::shared_ptr<RasterizerOutputStream> Process( std::shared_ptr<RasterizerInputStream> inputStream )
		{
			auto outputStream = std::make_shared<RasterizerOutputStream>();

			assert( m_renderingContext );
			std::shared_ptr<Texture> rt = m_renderingContext->GetRenderTaget( 0 );
			if ( rt == nullptr )
			{
				return outputStream;
			}

			int width = rt->Width();
			int height = rt->Height();

			// do scanline conversion
			size_t edgeCount = BuildEdgeTable( inputStream, width, height );
			if ( edgeCount > 0 )
			{
				size_t drawCount = GatherPixelsBeingDrawnFromScanlines( outputStream, width, height );
			}

			return outputStream;
		}

	private:
		struct EdgeTableElement
		{
			unsigned short YMax;
			unsigned short XMin;
			float Slope;
			// float PZ;	// depth

			const Color32& ColorL;
			const Color32& ColorR;
		};	

		struct ActiveEdgeTableElement
		{
			const EdgeTableElement& ETElement;
			float CurrentX;

			ActiveEdgeTableElement( const EdgeTableElement& etElem ) :
				ETElement( etElem ),
				CurrentX( etElem.XMin )
			{
			}

			// sortable by XMin ascending order
			bool operator<( const ActiveEdgeTableElement& rhs )
			{
				return ETElement.XMin < rhs.ETElement.XMin;
			}
		};	

		size_t BuildEdgeTable( std::shared_ptr<RasterizerInputStream> inputStream, int width, int height )
		{
			assert( inputStream );

			size_t numVertices = inputStream->Size();

			// ignore uncirculated vertices
			if ( numVertices < 3 )
			{
				return 0;
			}

			size_t edgeCount = 0;
			
			// reserve scanlines
			m_edgeTable.resize( height );

			size_t lastVertIndex = numVertices - 1;
			for ( size_t i = 0; i < numVertices; ++i )
			{
				// select an edge
				size_t v1Index = ( i == lastVertIndex ) ? 0 : i + 1;				
				const auto& v0 = inputStream->GetData( i );
				const auto& v1 = inputStream->GetData( v1Index );

				// make an ET element
				float yMax = 0.0f;
				float yMin = 0.0f;
				if ( v0.Y >= v1.Y )
				{
					yMax = v0.Y;
					yMin = v1.Y;
				}
				else
				{
					yMax = v1.Y;
					yMin = v0.Y;
				}

				EdgeTableElement elem
				{ 
					// YMax
					FloatToInteger<float, unsigned short>( yMax ),
					// XMin
					FloatToInteger<float, unsigned short>( Min<float>( v0.X, v1.X ) ), 
					// Slope
					0.0f,
					// Colors
					v0.Color,
					v1.Color,
				};

				float dx = v0.X - v1.X;
				float dy = v0.Y - v1.Y;
				elem.Slope = ( dx == 0.0f ) ? 0.0f : dy / dx;

				// select a start scanline with Y-axis clipping
				unsigned short scanline = FloatToInteger<float, unsigned short>( yMin );
				scanline = Clamp<unsigned short>( scanline, 0, height - 1 );

				// push this element at the selected scanline
				m_edgeTable[scanline].push_back( elem );

				++edgeCount;
			}

			return edgeCount;
		}

		size_t GatherPixelsBeingDrawnFromScanlines( std::shared_ptr<RasterizerOutputStream> outputStream, int width, int height )
		{
			assert( outputStream );
			assert( height == m_edgeTable.size() && "target height and scanline are mismatched" );

			size_t drawnCount = 0;

			// active edge table
			std::list<ActiveEdgeTableElement> aet;

			// for each scanline
			for ( size_t y = 0; y < height; ++y )
			{
				// find outside elements in AET and remove them
				auto iter = aet.begin();
				while ( iter != aet.end() )
				{
					const ActiveEdgeTableElement& elem = *iter;
					if ( elem.ETElement.YMax > height )
					{
						iter = aet.erase( iter );
					}
					else
					{
						++iter;
					}
				}

				// for each ET element
				const std::list<EdgeTableElement>& edgeList = m_edgeTable[y];
				for ( const auto& elem : edgeList )
				{
					// is inside?
					if ( elem.YMax <= height )
					{
						aet.emplace_back( elem );
					}
				}				
				aet.sort();

				// for each AET element
				for ( auto iter = aet.begin(); iter != aet.end(); ++iter )
				{
					// select left (=current) and right (=next) elements
					ActiveEdgeTableElement& elemLeft = *iter;
					if ( ++iter == aet.end() )
					{
						break;
					}
					ActiveEdgeTableElement& elemRight = *iter;

					// approximate the intersection of x-coordinates
					unsigned short xLeft = FloatToInteger<float, unsigned short>( elemLeft.CurrentX );
					unsigned short xRight = FloatToInteger<float, unsigned short>( elemRight.CurrentX );

					if ( xLeft > xRight )
					{
						Swap<unsigned short>( xLeft, xRight );
					}

					// clipping on RT
					xLeft = Min<unsigned short>( xLeft, 0 );
					xRight = Min<unsigned short>( xRight, width );

					// push inside pixels into the output stream from left-x to right-x
					for ( unsigned short x = xLeft; x < xRight; ++x )
					{
						__TODO( write the pixel Z value );
						__TODO( interpolate colors );
						outputStream->Push( x, y, 0.0f, elemLeft.ETElement.ColorL );

						++drawnCount;
					}

					// update the next position incrementally
					elemLeft.CurrentX += elemLeft.ETElement.Slope;
					elemRight.CurrentX += elemRight.ETElement.Slope;
				}
			}

			m_edgeTable.clear();

			return drawnCount;
		}

	private:
		std::shared_ptr<RenderingContext> m_renderingContext;
		std::vector<std::list<EdgeTableElement>> m_edgeTable;
	};


	/* class PixelProcessor
	*/
	class PixelProcessor : public IRenderingProcessor<PixelProcInputStream, PixelProcOutputStream>
	{
		NONCOPYABLE_CLASS( PixelProcessor )

	public:
		explicit PixelProcessor( RenderingContext* pRenderingContext ) :
			m_renderingContext( pRenderingContext )
		{
		}

		virtual ~PixelProcessor()
		{
		}
		
		virtual std::shared_ptr<PixelProcOutputStream> Process( std::shared_ptr<PixelProcInputStream> inputStream )
		{
			auto outputStream = std::make_shared<PixelProcOutputStream>();

			assert( m_renderingContext );
			std::shared_ptr<Texture> rt = m_renderingContext->GetRenderTaget( 0 );
			if ( rt == nullptr )
			{
				return outputStream;
			}

			assert( inputStream );
			size_t inputStreamSize = inputStream->Size();
			for ( size_t i = 0; i < inputStreamSize; ++i )
			{
				const auto& fragment = inputStream->GetData( i );

				// TODO: Z-buffering

				LockGuardPtr<Texture> guard( rt );
				if ( void* p = guard.Ptr() )
				{
					// TODO: pixel shading
					rt->WriteTexel( fragment.PX, fragment.PY, fragment.Color.Value );
				}				
			}

			return outputStream;
		}

	private:
		std::shared_ptr<RenderingContext> m_renderingContext;
	};


	/* class OutputMerger
	*/
	class OutputMerger : public IRenderingProcessor<OutputMergerInputStream, OutputMergerOutputStream>
	{
		NONCOPYABLE_CLASS( OutputMerger )

	public:
		explicit OutputMerger( RenderingContext* pRenderingContext ) :
			m_renderingContext( pRenderingContext )
		{
		}

		virtual ~OutputMerger()
		{
		}

		virtual std::shared_ptr<OutputMergerOutputStream> Process( std::shared_ptr<OutputMergerInputStream> inputStream )
		{
			//OutputMergerInputStream* pOut = new OutputMergerInputStream();

			//inputStream.reset();
			//return std::shared_ptr<OutputMergerInputStream>( pOut );
			return inputStream;
		}

	private:
		std::shared_ptr<RenderingContext> m_renderingContext;
	};



	/* class RenderingContext
	*/
	RenderingContext::RenderingContext( size_t numRenderTargets ) :
		m_inputAssembler( std::make_unique<InputAssembler>( this ) ),
		m_vertexProcessor( std::make_unique<VertexProcessor>( this ) ),
		m_rasterizer( std::make_unique<Rasterizer>( this ) ),
		m_pixelProcessor( std::make_unique<PixelProcessor>( this ) ),
		m_outputMerger( std::make_unique<OutputMerger>( this ) )
	{
		m_renderTargets.resize( numRenderTargets );
		/* nullptr initialization is not necessary
		for ( auto& rt : m_renderTargets )
		{
			rt = nullptr;
		}*/
	}

	void RenderingContext::Clear( byte r, byte g, byte b, byte a )
	{
		//unsigned int color = ( r | ( ((unsigned short) g) << 8 ) | ( ((unsigned int) b) << 16 )) | ( ((unsigned int) a) << 24 );

		size_t num = m_renderTargets.size();
		for ( size_t i = 0; i < num; ++i )
		{
			if ( m_renderTargets[i] == nullptr )
			{
				continue;
			}

			LockGuardPtr<Texture> guard( m_renderTargets[i] );
			if ( void* ptr = guard.Ptr() )
			{
				assert( ptr );

				std::shared_ptr<Texture> rt = m_renderTargets[i];

				int width = rt->Width();
				int height = rt->Height();

				if ( r == g && g == b )
				{
					int size = width * height * ComputeBytesPerPixel( rt->Format() );
					memset( ptr, r, size );
					return;
				}
					
				for ( int h = 0; h < height; ++h )
				{
					for ( int w = 0; w < width; ++w )
					{
						rt->WriteTexel( w, h, r, g, b );
					}
				}
			}
		}
	}

	void RenderingContext::Draw( std::shared_ptr<Mesh> mesh )
	{
		assert( m_inputAssembler );
		assert( m_vertexProcessor );
		assert( m_rasterizer );
		assert( m_pixelProcessor );
		assert( m_outputMerger );

		// input assembler
		std::shared_ptr<VertexProcInputStream> vpInput = m_inputAssembler->Process( mesh );

		// vertex processor
		std::shared_ptr<RasterizerInputStream> raInput = m_vertexProcessor->Process( vpInput );

		// rasterizer
		std::shared_ptr<PixelProcInputStream> ppInput = m_rasterizer->Process( raInput );

		// pixel processor
		std::shared_ptr<OutputMergerInputStream> omInput = m_pixelProcessor->Process( ppInput );

		// output merger
		omInput = m_outputMerger->Process( omInput );
	}


	/* RenderingDevice
	*/
	std::shared_ptr<RenderingContext> RenderingDevice::CreateRenderingContext()
	{
		auto context = std::make_shared<RenderingContext>( 4 /* the number of render targets */ );
		m_renderingContexts.push_back( context );
		return context;
	}
}
