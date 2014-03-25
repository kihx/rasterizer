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
		// Cannot use std::make_shared<>() by the protected access modifier.
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
			texture->m_pMemory = malloc( width * height * GetBytesPerPixel( format ) );
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

		PrimitiveType GetPrimitiveType() const
		{
			return m_primType;
		}

		void SetPrimitiveType( PrimitiveType primType )
		{
			m_primType = primType;
		}

	private:
		PrimitiveType m_primType;
	};


	struct PixelProcData
	{
		// x and y coordinates of a pixel
		unsigned short PX;
		unsigned short PY;
		
		// z of a pixel
		float Depth;

		// interpolated color
		Color32 Color;

		PixelProcData() :
			PX( 0 ),
			PY( 0 ),
			Depth( 1.0f ) // farthest
		{
		}

		PixelProcData( unsigned short px, unsigned short py, float Depth, const Color32& color ) :
			PX( px ),
			PY( py ),
			Depth( Depth ),
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

#define FACE_SPECIFIC		1

	/* class InputAssembler
	*/
	class InputAssembler : public IRenderingProcessor<Mesh, InputAssemblerOutputStream>
	{
		NONCOPYABLE_CLASS( InputAssembler )

	public:
		explicit InputAssembler( RenderingContext* pRenderingContext ) :
			m_pRenderingContext( pRenderingContext )
		{
		}

		virtual ~InputAssembler()
		{
		}

		virtual std::shared_ptr<InputAssemblerOutputStream> Process( std::shared_ptr<Mesh> inputStream )
		{
			auto outputStream = std::make_shared<InputAssemblerOutputStream>();

#ifdef SUPPORT_MSH
			// Count the number of vertices in the mesh, 
			// and reserve space for the output stream.
			size_t capacity = 0;
#ifdef FACE_SPECIFIC
			capacity += inputStream->NumVerticesInFace( m_faceIndex );
#else
			size_t numFaces = inputStream->NumFaces();
			for ( size_t face = 0; face < numFaces; ++face )
			{
				capacity += inputStream->NumVerticesInFace( face );
			}
#endif
			outputStream->Reserve( capacity );

			// Convert the mesh to an input stream of the vertex processor.
#ifdef FACE_SPECIFIC
			size_t face = m_faceIndex;
#else
			for ( size_t face = 0; face < numFaces; ++face )
#endif
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
		RenderingContext* m_pRenderingContext;

#ifdef FACE_SPECIFIC
	public:
		size_t m_faceIndex;
#endif
	};
	

	/* class VertexProcessor
	*/
	class VertexProcessor : public IRenderingProcessor<VertexProcInputStream, VertexProcOutputStream>
	{
		NONCOPYABLE_CLASS( VertexProcessor )

	public:
		explicit VertexProcessor( RenderingContext* pRenderingContext ) :
			m_pRenderingContext( pRenderingContext )
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

	private:
		RenderingContext* m_pRenderingContext;
	};

	
	/* class Rasterizer
	*/
	class Rasterizer : public IRenderingProcessor<RasterizerInputStream, RasterizerOutputStream>
	{
		NONCOPYABLE_CLASS( Rasterizer )

	public:
		explicit Rasterizer( RenderingContext* pRenderingContext ) :
			m_pRenderingContext( pRenderingContext )
		{
		}

		virtual ~Rasterizer()
		{
		}

		virtual std::shared_ptr<RasterizerOutputStream> Process( std::shared_ptr<RasterizerInputStream> inputStream )
		{
			auto outputStream = std::make_shared<RasterizerOutputStream>();

			assert( m_pRenderingContext );
			std::shared_ptr<Texture> rt = m_pRenderingContext->GetRenderTaget( 0 );
			if ( rt == nullptr )
			{
				return outputStream;
			}

			unsigned short width = static_cast<unsigned short>( rt->Width() );
			unsigned short height = static_cast<unsigned short>( rt->Height() );

			DoScanlineConversion( inputStream, outputStream, width, height );

			return outputStream;
		}

	private:
		struct EdgeTableElement
		{
			float YMax;
			float XMin;
			float XMax;
			float Slope;
			// float Depth;	// depth

			const Color32& ColorL;
			const Color32& ColorR;

			EdgeTableElement() = delete;
			EdgeTableElement& operator=( const EdgeTableElement& ) = delete;

			explicit EdgeTableElement( float yMax, float xMin, float xMax, float slope, const Color32& colorL, const Color32& colorR ) :
				YMax( yMax ),
				XMin( xMin ),
				XMax( xMax ),
				Slope( slope ),
				ColorL( colorL ),
				ColorR( colorR )
			{
			}
		};	

		struct ActiveEdgeTableElement
		{
			const EdgeTableElement& ETElement;
			float CurrentX;

			ActiveEdgeTableElement() = delete;
			ActiveEdgeTableElement& operator=( const ActiveEdgeTableElement& ) = delete;

			explicit ActiveEdgeTableElement( const EdgeTableElement& etElem ) :
				ETElement( etElem ),
				CurrentX( etElem.Slope > 0.0f ? etElem.XMin : etElem.XMax )
			{
			}

			// sort by x-less
			bool operator<( const ActiveEdgeTableElement& rhs )
			{
				return CurrentX < rhs.CurrentX;
			}
		};			

		void DoScanlineConversion( std::shared_ptr<RasterizerInputStream> inputStream, std::shared_ptr<RasterizerOutputStream> outputStream, unsigned short width, unsigned short height )
		{
			assert( inputStream );
			
			PrimitiveType primitiveType = inputStream->GetPrimitiveType();
			size_t numVerticesPerPrimitive = ComputeNumberOfVerticesPerPrimitive( primitiveType );

			// Validate the number of vertices considering primitive type.
			size_t numVertices = inputStream->Size();
			if ( numVertices < numVerticesPerPrimitive )
			{
				return;
			}
			else if ( numVertices % numVerticesPerPrimitive != 0 )
			{				
				LOG_WARNING( "incomplete primitive" );
				return;
			}

			// Reserve ET space based on scanlines.
			m_edgeTable.clear();
			m_edgeTable.resize( height );

			// Build an edge table by traversing each primitive in vertices,
			// and draw each primitive.
			size_t numPrimitives = numVertices / numVerticesPerPrimitive;

			// for each primitive
			for ( size_t p = 0; p < numPrimitives; ++p )
			{
				// edge: v0 -> v1
				// v0 = i th vertex
				// v1 = (i + 1) th vertex
				//
				// But if v1 is a vertex of the next primitive,
				// we must hold the first vertex of the current primitive to v1.
				// To keep in simple, we step last -> first -> second -> third ... and last - 1.
				size_t v1Index = p * numVerticesPerPrimitive;
				size_t v0Index = v1Index + ( numVerticesPerPrimitive - 1 );

				// for each vertex
				for ( size_t v = 0; v < numVerticesPerPrimitive; ++v )
				{
					const auto& v0 = inputStream->GetData( v0Index );
					const auto& v1 = inputStream->GetData( v1Index );

					// Make an ET element.
					float xMax = v0.X;
					float xMin = v1.X;
					if ( v0.X < v1.X )
					{
						Swap<float>( xMax, xMin );
					}

					float yMax = v0.Y;
					float yMin = v1.Y;
					if ( v0.Y < v1.Y )
					{
						Swap<float>( yMax, yMin );
					}

					float dx = v0.X - v1.X;
					float dy = v0.Y - v1.Y;
					float slope = ( dy == 0.0f ) ? 0.0f : ( dx / dy );

					// Select a start scanline with Y-axis clipping.
					unsigned short startY = FloatToInteger<float, unsigned short>( std::round( yMin ) );
					startY = Clamp<unsigned short>( startY, 0, height - 1 );

					// Push this element at the selected scanline.
					m_edgeTable[startY].emplace_back( yMax, xMin, xMax, slope, v0.Color, v1.Color );	// is this color order ok?				

					// Update next indices
					++v1Index;
					v0Index = v1Index - 1;
				}

				// rasterization
				GatherPixelsBeingDrawnFromScanlines( outputStream, width, height );
			}
		}

		void GatherPixelsBeingDrawnFromScanlines( std::shared_ptr<RasterizerOutputStream> outputStream, unsigned short width, unsigned short height )
		{
			assert( outputStream );
			assert( height == m_edgeTable.size() && "target height and scanline are mismatched" );
			
			std::list<ActiveEdgeTableElement> aet;

			// for each scanline
			for ( unsigned short y = 0; y < height; ++y )
			{
				// Find outside elements in the AET and remove them.
				auto iter = aet.begin();
				while ( iter != aet.end() )
				{
					const ActiveEdgeTableElement& elem = *iter;
					if ( y >= elem.ETElement.YMax )
					{
						iter = aet.erase( iter );
					}
					else
					{
						++iter;
					}
				}

				// Push inside ET elements on this scanline into the AET.
				const std::list<EdgeTableElement>& edgeList = m_edgeTable[y];
				for ( const auto& elem : edgeList )
				{
					if ( y < elem.YMax )
					{
						aet.emplace_back( elem );
					}
				}				

				// Sort AET elements from left to right.
				aet.sort();

				// Gather pixels being drawn using the AET.
				for ( auto iter = aet.begin(); iter != aet.end(); ++iter )
				{
					// First, select left (=current) and right (=next) edge elements.
					ActiveEdgeTableElement& elemLeft = *iter;
					if ( ++iter == aet.end() )
					{
						break;
					}
					ActiveEdgeTableElement& elemRight = *iter;

					// Approximate intersection pixels betweeen each edge and the scanline.
					unsigned short xLeft = FloatToInteger<float, unsigned short>( std::round( elemLeft.CurrentX ) );
					unsigned short xRight = FloatToInteger<float, unsigned short>( std::round( elemRight.CurrentX ) );

					// clipping on RT
					xLeft = Max<unsigned short>( xLeft, 0 );
					xRight = Min<unsigned short>( xRight, width );

					// Push inside pixels between intersections into the output stream.
					for ( unsigned short x = xLeft; x < xRight; ++x )
					{
						__TODO( write the pixel Z value );
						__TODO( interpolate colors );
						outputStream->Push( x, y, 0.0f, elemLeft.ETElement.ColorL );
					}

					// Update the next position incrementally
					elemLeft.CurrentX += elemLeft.ETElement.Slope;
					elemRight.CurrentX += elemRight.ETElement.Slope;
				}
			}

			for ( unsigned short y = 0; y < height; ++y )
			{
				m_edgeTable[y].clear();
			}
		}

	private:
		RenderingContext* m_pRenderingContext;
		std::vector<std::list<EdgeTableElement>> m_edgeTable;
	};


	/* class PixelProcessor
	*/
	class PixelProcessor : public IRenderingProcessor<PixelProcInputStream, PixelProcOutputStream>
	{
		NONCOPYABLE_CLASS( PixelProcessor )

	public:
		explicit PixelProcessor( RenderingContext* pRenderingContext ) :
			m_pRenderingContext( pRenderingContext )
		{
		}

		virtual ~PixelProcessor()
		{
		}
		
		virtual std::shared_ptr<PixelProcOutputStream> Process( std::shared_ptr<PixelProcInputStream> inputStream )
		{
			auto outputStream = std::make_shared<PixelProcOutputStream>();

			assert( m_pRenderingContext );
			std::shared_ptr<Texture> rt = m_pRenderingContext->GetRenderTaget( 0 );
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
		RenderingContext* m_pRenderingContext;
	};


	/* class OutputMerger
	*/
	class OutputMerger : public IRenderingProcessor<OutputMergerInputStream, OutputMergerOutputStream>
	{
		NONCOPYABLE_CLASS( OutputMerger )

	public:
		explicit OutputMerger( RenderingContext* pRenderingContext ) :
			m_pRenderingContext( pRenderingContext )
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
		RenderingContext* m_pRenderingContext;
	};



	/* class RenderingContext
	*/
	RenderingContext::RenderingContext( size_t numRenderTargets ) :
		m_inputAssembler( std::make_shared<InputAssembler>( this ) ),
		m_vertexProcessor( std::make_shared<VertexProcessor>( this ) ),
		m_rasterizer( std::make_shared<Rasterizer>( this ) ),
		m_pixelProcessor( std::make_shared<PixelProcessor>( this ) ),
		m_outputMerger( std::make_shared<OutputMerger>( this ) )
	{
		m_renderTargets.resize( numRenderTargets );
		/* nullptr initialization is not necessary.
		for ( auto& rt : m_renderTargets )
		{
			rt = nullptr;
		}*/
	}

	void RenderingContext::Clear( byte r, byte g, byte b, byte a )
	{
		//unsigned int color = ( r | ( ((unsigned short) g) << 8 ) | ( ((unsigned int) b) << 16 )) | ( ((unsigned int) a) << 24 );

		a = 255;	// FIXME

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
					int size = width * height * GetBytesPerPixel( rt->Format() );
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
		if ( mesh == nullptr )
		{
			return;
		}

		assert( m_inputAssembler );
		assert( m_vertexProcessor );
		assert( m_rasterizer );
		assert( m_pixelProcessor );
		assert( m_outputMerger );

#ifdef FACE_SPECIFIC
		for ( size_t face = 0; face < mesh->NumFaces(); ++face )
		{		
			m_inputAssembler->m_faceIndex = face;
#else
		// If a source is the Mesh class
		// we assume that the primitive type is 'triangles'.
		PrimitiveType primType = PrimitiveType::TRIANGLES;
#endif
		// input assembler
		std::shared_ptr<VertexProcInputStream> vpInput = m_inputAssembler->Process( mesh );

		printf( "VertexProcInputStream Size: %d\n", vpInput->Size() );

		// vertex processor
		std::shared_ptr<RasterizerInputStream> raInput = m_vertexProcessor->Process( vpInput );

		printf( "RasterizerInputStream Size: %d\n", raInput->Size() );

		// rasterizer
#ifdef FACE_SPECIFIC
		size_t numVerticesInFace = mesh->NumVerticesInFace( face );
		raInput->SetPrimitiveType( GetPrimitiveTypeFromNumberOfVertices( numVerticesInFace ) );
#else
		raInput->SetPrimitiveType( primType );
#endif
		std::shared_ptr<PixelProcInputStream> ppInput = m_rasterizer->Process( raInput );

		printf( "PixelProcInputStream Size: %d\n", ppInput->Size() );

		// pixel processor
		std::shared_ptr<OutputMergerInputStream> omInput = m_pixelProcessor->Process( ppInput );

		// output merger
		omInput = m_outputMerger->Process( omInput );
#ifdef FACE_SPECIFIC
		}
#endif
	}


	/* RenderingDevice
	*/
	std::shared_ptr<RenderingContext> RenderingDevice::CreateRenderingContext()
	{
		auto context = std::make_shared<RenderingContext>( 4 /* the number of render targets */ );
		m_renderingContexts.emplace_back( context );
		return context;
	}
}
