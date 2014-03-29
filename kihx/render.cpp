#include "stdafx.h"
#include "render.h"
#include "mesh.h"
#include "texture.h"
#include "vector.h"
#include "matrix.h"
#include "random.h"

#include <list>


namespace kih
{	
	struct VertexProcData
	{		
		Vector3 Position;		// object-space position
		//Color32 Color;		// vertex color

		VertexProcData() = default;

		VertexProcData( const float position[3]/*, const Color32& color*/ ) :
			Position( position )/*,
			Color( color )*/
		{
		}

		VertexProcData( const Vector3& position/*, const Color32& color*/ ) :
			Position( position )/*,
			Color( color )*/
		{
		}

		VertexProcData( const VertexProcData& data ) = default;
	};

	/* class VertexProcInputStream
	*/
	class VertexProcInputStream : public BaseInputOutputStream<VertexProcData>
	{
	public:
		VertexProcInputStream() :
			m_coordinatesType( CoordinatesType::Projective )
		{
		}

		CoordinatesType GetCoordinatesType() const
		{
			return m_coordinatesType;
		}

		void SetCoordinatesType( CoordinatesType coord )
		{
			m_coordinatesType = coord;
		}

	private:
		CoordinatesType m_coordinatesType;
	};


	struct RasterizerData
	{		
		Vector4 Position;	// wvp transformed position		
		//Color32 Color;		// vertex color

		RasterizerData() = default;

		RasterizerData( const Vector3& position/*, const Color32& color*/ ) :
			Position( position )/*,
			Color( color )*/
		{
		}

		RasterizerData( const Vector4& position ) :
			Position( position )
		{
		}

		RasterizerData( const RasterizerData& data ) = default;
	};

	/* class RasterizerInputStream
	*/
	class RasterizerInputStream : public BaseInputOutputStream<RasterizerData>
	{
	public:
		RasterizerInputStream() :
			m_coordinatesType( CoordinatesType::Projective ),
			m_primitiveType( PrimitiveType::Triangles )
		{
		}

		CoordinatesType GetCoordinatesType() const
		{
			return m_coordinatesType;
		}

		void SetCoordinatesType( CoordinatesType coord )
		{
			m_coordinatesType = coord;
		}

		PrimitiveType GetPrimitiveType() const
		{
			return m_primitiveType;
		}

		void SetPrimitiveType( PrimitiveType primType )
		{
			m_primitiveType = primType;
		}

	private:
		CoordinatesType m_coordinatesType;
		PrimitiveType m_primitiveType;
	};


	struct PixelProcData
	{
		// x and y coordinates of a pixel
		unsigned short PX;
		unsigned short PY;
		
		// z of a pixel
		float Depth;
		
		Color32 Color;		// interpolated color

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
	class OutputMergerInputStream : public BaseInputOutputStream<PixelProcData>
	{
	public:
		OutputMergerInputStream()
		{
		}

	private:

	};


	/* class InputAssembler
	*/
	class InputAssembler : public BaseRenderingProcessor<IMesh, InputAssemblerOutputStream>
	{
		NONCOPYABLE_CLASS( InputAssembler )

	public:
		explicit InputAssembler( RenderingContext* pRenderingContext ) :
			BaseRenderingProcessor( pRenderingContext ),
			m_faceIndex( 0 )
		{
		}

		virtual ~InputAssembler()
		{
		}

		virtual std::shared_ptr<InputAssemblerOutputStream> Process( std::shared_ptr<IMesh> inputStream )
		{
			assert( inputStream );

			m_outputStream->Clear();
			
			if ( inputStream->GetPrimitiveType() == PrimitiveType::Undefined )
			{
				const IrregularMesh* mesh = dynamic_cast< IrregularMesh* >( inputStream.get() );
				if ( mesh == nullptr )
				{
					return m_outputStream;
				}

				// per face rendering
				size_t capacity = mesh->NumVerticesInFace( m_faceIndex );
				m_outputStream->Reserve( capacity );

				// Convert the face geometry to an input stream of the vertex processor.
				size_t face = m_faceIndex;
				size_t numVertices = mesh->NumVerticesInFace( face );
				for ( size_t vert = 0; vert < numVertices; ++vert )
				{
					m_outputStream->Push( mesh->GetVertexInFaceAt( face, vert ) );
				}
			}
			else
			{
				const OptimizedMesh* mesh = dynamic_cast< OptimizedMesh* >( inputStream.get() );
				if ( mesh == nullptr )
				{
					return m_outputStream;
				}
				
				const auto& vertexBuffer = mesh->GetVertexBufferConst();
				const auto& indexBuffer = mesh->GetIndexBufferConst();

				// per mesh rendering
				size_t numIndices = indexBuffer.Size();
				m_outputStream->Reserve( numIndices );

				// Convert the mesh geometry to an input stream of the vertex processor.
				for ( size_t i = 0; i < numIndices; ++i )
				{
					unsigned short index = indexBuffer.GetIndexConst( i ) - 1;
					m_outputStream->Push( &vertexBuffer.GetVertexConst( index ).x );
				}
			}

			m_outputStream->SetCoordinatesType( inputStream->GetCoordinatesType() );

			return m_outputStream;
		}

		void SetFaceIndex( size_t index )
		{
			m_faceIndex = index;
		}

	private:
		size_t m_faceIndex;
	};
	

	/* class VertexProcessor
	*/
	class VertexProcessor : public BaseRenderingProcessor<VertexProcInputStream, VertexProcOutputStream>
	{
		NONCOPYABLE_CLASS( VertexProcessor )

	public:
		explicit VertexProcessor( RenderingContext* pRenderingContext ) :
			BaseRenderingProcessor( pRenderingContext )
		{
		}

		virtual ~VertexProcessor()
		{
		}

		virtual std::shared_ptr<VertexProcOutputStream> Process( std::shared_ptr<VertexProcInputStream> inputStream )
		{
			assert( inputStream );

			m_outputStream->Clear();

			size_t inputSize = inputStream->Size();
			if ( inputSize > 0 )
			{
				// Currently, input and output size is always same.
				m_outputStream->Reserve( inputSize );

				if ( inputStream->GetCoordinatesType() == CoordinatesType::ReciprocalHomogeneous )
				{
					for ( size_t i = 0; i < inputSize; ++i )
					{
						const auto& vertex = inputStream->GetData( i );

						m_outputStream->Push( vertex.Position );
					}
				}
				else
				{
					const Matrix4& wvp = GetSharedConstantBuffer().GetMatrix4( ConstantBuffer::WVPMatrix );

					for ( size_t i = 0; i < inputSize; ++i )
					{
						const auto& vertex = inputStream->GetData( i );

						Vector4 hPos;
						TransformWVP( vertex.Position, wvp, hPos );

						m_outputStream->Push( hPos );
					}
				}
			}

			m_outputStream->SetCoordinatesType( inputStream->GetCoordinatesType() );

			return m_outputStream;
		}
		
	private:
		// WVP transform
		void TransformWVP( const Vector3& position, const Matrix4& wvp, Vector4& outPosition )
		{
			outPosition = Vector3_Transform( position, wvp );

			//printf( "hpos: %.2f %.2f %.2f\n", hpos.X, hpos.Y, hpos.Z );
		}
	};

	
	/* class Rasterizer
	*/
	class Rasterizer : public BaseRenderingProcessor<RasterizerInputStream, RasterizerOutputStream>
	{
		NONCOPYABLE_CLASS( Rasterizer )

	public:
		explicit Rasterizer( RenderingContext* pRenderingContext ) :
			BaseRenderingProcessor( pRenderingContext )
		{
		}

		virtual ~Rasterizer()
		{
		}

		virtual std::shared_ptr<RasterizerOutputStream> Process( std::shared_ptr<RasterizerInputStream> inputStream )
		{
			assert( inputStream );

			m_outputStream->Clear();

			std::shared_ptr<Texture> rt = GetContext()->GetRenderTaget( 0 );
			if ( rt == nullptr )
			{
				return m_outputStream;
			}

			unsigned short width = static_cast<unsigned short>( rt->Width() );
			unsigned short height = static_cast<unsigned short>( rt->Height() );

			assert( ( width > 0 && height > 0 ) && "invalid operation" );

			TransformViewport( inputStream, width, height );

			// FIXME: is this ok??
			m_outputStream->Reserve( width * height );
			DoScanlineConversion( inputStream, m_outputStream, width, height );

			return m_outputStream;
		}
	
	private:
		struct EdgeTableElement
		{
			float YMax;
			float XMin;
			float XMax;
			float Slope;
			// float Depth;	// depth

			//const Color32& ColorL;
			//const Color32& ColorR;

			EdgeTableElement() = delete;

			explicit EdgeTableElement( float yMax, float xMin, float xMax, float slope/*, const Color32& colorL, const Color32& colorR*/ ) :
				YMax( yMax ),
				XMin( xMin ),
				XMax( xMax ),
				Slope( slope )/*,
				ColorL( colorL ),
				ColorR( colorR )*/
			{
			}

			EdgeTableElement& operator=( const EdgeTableElement& ) = delete;
		};	

		struct ActiveEdgeTableElement
		{
			const EdgeTableElement& ETElement;
			float CurrentX;

			ActiveEdgeTableElement() = delete;

			explicit ActiveEdgeTableElement( const EdgeTableElement& etElem ) :
				ETElement( etElem ),
				CurrentX( etElem.Slope > 0.0f ? etElem.XMin : etElem.XMax )
			{
			}

			ActiveEdgeTableElement& operator=( const ActiveEdgeTableElement& ) = delete;

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
			size_t numVerticesPerPrimitive = GetNumberOfVerticesPerPrimitive( primitiveType );

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
					float xMax = v0.Position.X;
					float xMin = v1.Position.X;
					if ( v0.Position.X < v1.Position.X )
					{
						Swap<float>( xMax, xMin );
					}

					float yMax = v0.Position.Y;
					float yMin = v1.Position.Y;
					if ( v0.Position.Y < v1.Position.Y )
					{
						Swap<float>( yMax, yMin );
					}

					float dx = v0.Position.X - v1.Position.X;
					float dy = v0.Position.Y - v1.Position.Y;
					float slope = ( dy == 0.0f ) ? 0.0f : ( dx / dy );

					// Select a start scanline with Y-axis clipping.
					unsigned short startY = FloatToInteger<float, unsigned short>( std::round( yMin ) );
					startY = Clamp<unsigned short>( startY, 0, height - 1 );

					// Push this element at the selected scanline.
					m_edgeTable[startY].emplace_back( yMax, xMin, xMax, slope );

					// Update next indices.
					v0Index = v1Index;
					++v1Index;
				}

				// rasterization
				GatherPixelsBeingDrawnFromScanlines( outputStream, width, height );
			}
		}

		void GatherPixelsBeingDrawnFromScanlines( std::shared_ptr<RasterizerOutputStream> outputStream, unsigned short width, unsigned short height )
		{
			assert( outputStream );
			assert( height == m_edgeTable.size() && "target height and scanline are mismatched" );
			
			// random color for debugging
			byte seedR = Random::Next( 0, 255 );
			byte seedG = Random::Next( 0, 255 );
			byte seedB = Random::Next( 0, 255 );
			Color32 color( seedR, seedG, seedB, 255 );

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
				for ( const auto& elem : m_edgeTable[y] )
				{
					if ( y < elem.YMax )
					{
						aet.emplace_back( elem );
					}
				}

				if ( aet.empty() )
				{
					continue;
				}

				// Sort AET elements from left to right.	
				if ( aet.size() > 1 )
				{
					aet.sort();
				}

				// Gather pixels being drawn using the AET.
				for ( auto iter = aet.begin(); iter != aet.end(); ++iter )
				{
					// First, select left (=current) and right (=next) edge elements.
					ActiveEdgeTableElement& elemLeft = *iter;
					if ( ++iter == aet.end() )
					{
						elemLeft.CurrentX += elemLeft.ETElement.Slope;
						break;
					}
					ActiveEdgeTableElement& elemRight = *iter;

					// Approximate intersection pixels betweeen edges on the scanline.
					if ( elemLeft.CurrentX != elemRight.CurrentX )
					{
						unsigned short xLeft = FloatToInteger<float, unsigned short>( std::round( elemLeft.CurrentX ) );
						unsigned short xRight = FloatToInteger<float, unsigned short>( std::round( elemRight.CurrentX ) );

						// clipping on RT
						xLeft = Max<unsigned short>( xLeft, 0 );
						xRight = Min<unsigned short>( xRight, width );

						// Push inside pixels between intersections into the output stream.
						for ( unsigned short x = xLeft; x < xRight; ++x )
						{
							__TODO( write the pixel Z value );
							outputStream->Push( x, y, 0.0f, color );	
							//outputStream->Push( x, y, 0.0f/*, elemLeft.ETElement.ColorL*/ );	
						}
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

		void TransformViewport( std::shared_ptr<RasterizerInputStream> inputStream, unsigned short width, unsigned short height )
		{
			// viewport transform for projective coordinates
			if ( inputStream->GetCoordinatesType() != CoordinatesType::Projective )
			{
				return;
			}

			float factorX = width * 0.5f;
			float factorY = height * 0.5f;

			size_t numVertices = inputStream->Size();
			for ( size_t v = 0; v < numVertices; ++v )
			{
				auto& data = inputStream->GetData( v );

				// perspective division
				data.Position /= data.Position.W;

				// NDC -> viewport
				data.Position.X *= factorX;
				data.Position.X += factorX;	// + x

				data.Position.Y *= factorY;
				data.Position.Y += factorY;	// + y

				// [-1, 1] to [0, 1]
				data.Position.Z *= 0.5f;
				data.Position.Z += 0.5f;

				printf( "data: %.2f %.2f %.2f\n", data.Position.X, data.Position.Y, data.Position.Z );

				if ( (v + 1) % 3 == 0 )
				{
					printf( "\n" );
				}
			}
		}

	private:
		std::vector<std::list<EdgeTableElement>> m_edgeTable;
	};


	/* class PixelProcessor
	*/
	class PixelProcessor : public BaseRenderingProcessor<PixelProcInputStream, PixelProcOutputStream>
	{
		NONCOPYABLE_CLASS( PixelProcessor )

	public:
		explicit PixelProcessor( RenderingContext* pRenderingContext ) :
			BaseRenderingProcessor( pRenderingContext )
		{
		}

		virtual ~PixelProcessor()
		{
		}
		
		virtual std::shared_ptr<PixelProcOutputStream> Process( std::shared_ptr<PixelProcInputStream> inputStream )
		{
			assert( inputStream );

			m_outputStream->Clear();
			
			std::shared_ptr<Texture> rt = GetContext()->GetRenderTaget( 0 );
			if ( rt == nullptr )
			{
				return m_outputStream;
			}

			size_t inputStreamSize = inputStream->Size();
			if ( inputStreamSize > 0 )
			{
				LockGuardPtr<Texture> guard( rt );
				if ( byte* buffer = static_cast< byte* >( guard.Ptr() ) )
				{
					const Vector4& color = GetSharedConstantBuffer().GetVector4( ConstantBuffer::DiffuseColor );
					Color32 color32 = Vector4_ToColor32( color );

					int width = rt->Width();
					int stride = GetBytesPerPixel( rt->Format() );

					for ( size_t i = 0; i < inputStreamSize; ++i )
					{
						const auto& fragment = inputStream->GetData( i );

						color32 = fragment.Color;

						// TODO: Z-buffering

						// TODO: pixel shading
#if 1
						byte* base = buffer + ( ( ( width * fragment.PY ) + fragment.PX ) * stride );
						*( base ) = color32.R;
						*( base + 1 ) = color32.G;
						*( base + 2 ) = color32.B;

						//*( base /*+ 0*/ ) = fragment.Color.R;
						//*( base + 1 ) = fragment.Color.G;
						//*( base + 2 ) = fragment.Color.B;
#else
						//rt->WriteTexel( fragment.PX, fragment.PY, fragment.Color.Value );
#endif
					}
				}
			}

			return m_outputStream;
		}
	};


	/* class OutputMerger
	*/
	class OutputMerger : public BaseRenderingProcessor<OutputMergerInputStream, OutputMergerOutputStream>
	{
		NONCOPYABLE_CLASS( OutputMerger )

	public:
		explicit OutputMerger( RenderingContext* pRenderingContext ) :
			BaseRenderingProcessor( pRenderingContext )
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

	void RenderingContext::Draw( std::shared_ptr<IMesh> mesh )
	{
		if ( mesh == nullptr )
		{
			return;
		}

		RenderingDevice* pDevice = RenderingDevice::GetInstance();
		assert( pDevice );
		assert( m_inputAssembler );
		assert( m_vertexProcessor );
		assert( m_rasterizer );
		assert( m_pixelProcessor );
		assert( m_outputMerger );

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

	void RenderingContext::DrawInternal( std::shared_ptr<IMesh> mesh, int numVerticesPerPrimitive )
	{
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
		auto context = std::make_shared<RenderingContext>( 4 /* the number of render targets */ );
		m_renderingContexts.emplace_back( context );
		return context;
	}
}
