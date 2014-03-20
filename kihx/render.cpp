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
		Texture* pTexture = new Texture();
		pTexture->m_width = width;
		pTexture->m_height = height;
		pTexture->m_format = format;

		pTexture->m_flags = 0;

		if ( pExternalMemory )
		{
			pTexture->SetExternalMemory( pExternalMemory );
		}
		else
		{
			int bytesPerPixel = 3;
			pTexture->m_pMemory = malloc( width * height * bytesPerPixel );
		}

		return std::shared_ptr<Texture>( pTexture );
	}


	/* class VertexProcInputStream
	*/
	class VertexProcInputStream
	{
	public:
		struct Data
		{
			union
			{
				struct
				{
					// object-space position
					float X;
					float Y;
					float Z;

					// vertex color
					byte R;
					byte G;
					byte B;
					byte A;
				};

				float Position[3];
				byte Color[4];
			};

			Data() :
				X( 0.0f ),
				Y( 0.0f ),
				Z( 0.0f ),
				R( 0 ),
				G( 0 ),
				B( 0 ),
				A( 0 )
			{
			}

			Data( const float position[3], const byte color[4] ) :
				X( position[0] ),
				Y( position[1] ),
				Z( position[2] ),
				R( color[0] ),
				G( color[1] ),
				B( color[2] )
			{
			}

			//Data( const Data& data )
			//{
			//	Assign( data.Position, data.Color );
			//}

			void Assign( const float position[3], const byte color[4] )
			{
				for ( int i = 0; i < 3; ++i )
				{
					Position[i] = position[i];
				}

				for ( int i = 0; i < 4; ++i )
				{
					Color[i] = color[i];
				}
			}
		};

		VertexProcInputStream()
		{
		}

		template <typename... Args>
		void Push( Args&&... args )
		{
			m_streamSource.emplace_back( std::forward<Args>( args )... );
		}

		//void Push( const float position[3], const byte color[4] )
		//{
		//	m_streamSource.emplace_back( position, color );
		//}

		const float* GetData( size_t index ) const
		{
			assert( ( index >= 0 && index < Size() ) && "Out of ranged index" );
			return m_streamSource[index].Position;
		}

		const float* GetStreamSource() const
		{
			if ( m_streamSource.empty() )
			{
				return nullptr;
			}
			else
			{
				return &m_streamSource[0].X;
			}
		}

		size_t Size() const
		{
			return m_streamSource.size();
		}

		void Reserve( size_t capacity )
		{
			m_streamSource.reserve( capacity );
		}

	private:
		std::vector<Data> m_streamSource;
	};


	/* class RasterizerInputStream
	*/
	class RasterizerInputStream
	{
	public:
		struct Data
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

			Data() :
				X( 0.0f ),
				Y( 0.0f ),
				Z( 0.0f ),
				W( 1.0f )
			{
			}

			Data( const float position[4] ) :
				X( position[0] ),
				Y( position[1] ),
				Z( position[2] ),
				W( position[3] )
			{
			}

			//Data( const Data& data )
			//{
			//	const int DataSize = sizeof( Data );
			//	memcpy_s( this, DataSize, &data, DataSize );
			//}
		};

		RasterizerInputStream()
		{
		}

		template <typename... Args>
		void Push( Args&&... args )
		{
			m_streamSource.emplace_back( std::forward<Args>( args )... );
		}

		//void Push( const float position[4] )
		//{
		//	m_streamSource.emplace_back( position );
		//}

		const float* GetStreamSource() const
		{
			if ( m_streamSource.empty() )
			{
				return nullptr;
			}
			else
			{
				return &m_streamSource[0].X;
			}
		}

		const float* GetPosition( size_t index ) const
		{
			assert( ( index >= 0 && index < Size() ) && "Out of ranged index" );
			return m_streamSource[index].Position;
		}

		const Data& GetData( size_t index ) const
		{
			assert( ( index >= 0 && index < Size() ) && "Out of ranged index" );
			return m_streamSource[index];
		}

		size_t Size() const
		{
			return m_streamSource.size();
		}

		void Reserve( size_t capacity )
		{
			m_streamSource.reserve( capacity );
		}

	private:
		std::vector<Data> m_streamSource;
	};


	/* class PixelProcInputStream
	*/
	class PixelProcInputStream
	{
	public:
		struct Data
		{
			// x and y coordinates of a pixel
			unsigned short PX;
			unsigned short PY;
			// z of a pixel
			float PZ;

			Data() :
				PX( 0 ),
				PY( 0 ),
				PZ( 1.0f ) // farthest
			{
			}

			Data( unsigned short px, unsigned short py, float pz ) :
				PX( px ),
				PY( py ),
				PZ( pz )
			{
			}

			//Data( const Data& data )
			//{
			//	const int DataSize = sizeof( Data );
			//	memcpy_s( this, DataSize, &data, DataSize );
			//}
		};

		PixelProcInputStream()
		{
		}

		template <typename... Args>
		void Push( Args&&... args )
		{
			m_streamSource.emplace_back( std::forward<Args>( args )... );
		}

		//void Push( const float position[4] )
		//{
		//	m_streamSource.emplace_back( position );
		//}

		__UNDONE( change return type of const void* );
		const void* GetStreamSource() const
		{
			if ( m_streamSource.empty() )
			{
				return nullptr;
			}
			else
			{
				return &m_streamSource[0].PX;
			}
		}

		const Data& GetData( size_t index ) const
		{
			assert( ( index >= 0 && index < Size() ) && "Out of ranged index" );
			return m_streamSource[index];
		}

		size_t Size() const
		{
			return m_streamSource.size();
		}

		void Reserve( size_t capacity )
		{
			m_streamSource.reserve( capacity );
		}

	private:
		std::vector<Data> m_streamSource;
	};


	/* class OutputMergerInputStream
	*/
	class OutputMergerInputStream
	{
	public:
		OutputMergerInputStream()
		{
		}

	private:
		void* m_pRenderBuffer;
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
			InputAssemblerOutputStream* pOutputStream = new InputAssemblerOutputStream( );

			// Count the number of vertices in the mesh
#ifdef SUPPORT_MSH
			size_t capacity = 0;
			size_t numFaces = inputStream->NumFaces();
			for ( size_t face = 0; face < numFaces; ++face )
			{
				capacity += inputStream->NumVerticesInFace( face );
			}
			pOutputStream->Reserve( capacity );

			// Convert the mesh to an input stream of the vertex processor
			for ( size_t face = 0; face < numFaces; ++face )
			{
				const byte* color = inputStream->GetFaceColor( face );

				size_t numVertices = inputStream->NumVerticesInFace( face );
				for ( size_t vert = 0; vert < numVertices; ++vert )
				{
					__TODO( change vertex color to material color using a pixel processor constant buffer );
					pOutputStream->Push( inputStream->GetVertexInFaceAt( face, vert ), color );
				}
			}
#else
			static_assert( 0, "not implemented yet" );
#endif

			return std::shared_ptr<InputAssemblerOutputStream>( pOutputStream );
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
			VertexProcOutputStream* pOutputStream = new VertexProcOutputStream( );

			size_t inputSize = inputStream->Size();
			for ( size_t i = 0; i < inputSize; ++i )
			{
				float transformedPosition[4];
				Transform( inputStream->GetData( i ), transformedPosition );

				pOutputStream->Push( transformedPosition );
			}

			return std::shared_ptr<VertexProcOutputStream>( pOutputStream );
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


	/* class ScanlineConversionImpl
	*/
	class ScanlineConversionImpl
	{
	private:
		struct EdgeElement
		{
			unsigned short YMax;
			unsigned short XMin;
			float Slope;
		};

	public:
		ScanlineConversionImpl()
		{
		}

		~ScanlineConversionImpl()
		{
		}

	private:
		void BuildEdgeTable( std::shared_ptr<RasterizerInputStream> inputStream, std::shared_ptr<Texture> renderTarget )
		{
			assert( inputStream.get() && "nullptr input stream" );
			assert( renderTarget.get() && "nullptr render target" );

			size_t numVertices = inputStream->Size();

			// ignore uncirculated vertices
			if ( numVertices < 3 )
			{
				return;
			}

			// reserve scanlines
			int height = renderTarget->Height();
			m_edgeTable.resize( height );

			size_t lastVertIndex = numVertices - 1;
			for ( size_t i = 0; i < numVertices; ++i )
			{
				// select an edge
				size_t v1Index = ( i == lastVertIndex ) ? 0 : i + 1;				
				const RasterizerInputStream::Data& v0 = inputStream->GetData( i );
				const RasterizerInputStream::Data& v1 = inputStream->GetData( v1Index );

				// here, fill an ET element
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

				EdgeElement elem
				{ 
					// YMax
					FloatToInteger<float, unsigned short>( yMax ),
					// XMin
					FloatToInteger<float, unsigned short>( min( v0.X, v1.X ) ), 
					// Slope
					0.0f 
				};

				float dx = v0.X - v1.X;
				float dy = v0.Y - v1.Y;
				elem.Slope = ( dx == 0.0f ) ? 0.0f : dy / dx;

				// select a start scanline with Y-axis clipping
				unsigned short scanline = FloatToInteger<float, unsigned short>( yMin );
				scanline = Clamp<unsigned short>( scanline, 0, height - 1 );

				// add this element at the selected scanline
				m_edgeTable[scanline].push_back( elem );
			}
		}

	private:
		std::vector<std::list<EdgeElement>> m_edgeTable;
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
			RasterizerOutputStream* pOutputStream = new RasterizerOutputStream( );


			return std::shared_ptr<RasterizerOutputStream>( pOutputStream );
		}

	private:
		std::shared_ptr<RenderingContext> m_renderingContext;
		ScanlineConversionImpl m_scanlineImpl;
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
			PixelProcOutputStream* pOutputStream = new PixelProcOutputStream( );

			return std::shared_ptr<PixelProcOutputStream>( pOutputStream );
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
			if ( Texture* pTexture = m_renderTargets[i].get() )
			{
				LockGuard<Texture> guard( pTexture );
				if ( void* p = guard.Ptr() )
				{
					assert( p && "a nullptr pointer of internal memory of a texture" );

					if ( r == g && g == b )
					{
						int size = pTexture->Width() * pTexture->Height() * ComputeBytesPerPixel( pTexture->Format() );
						memset( p, r, size );
						return;
					}
					
					for ( int h = 0; h < pTexture->Height(); ++h )
					{
						for ( int w = 0; w < pTexture->Width(); ++w )
						{
							pTexture->WriteTexel( w, h, r, g, b );
						}
					}
				}
			}
		}
	}

	void RenderingContext::Draw( std::shared_ptr<Mesh> mesh )
	{
		assert( m_inputAssembler.get() && "nullptr of input assembler" );
		assert( m_vertexProcessor.get( ) && "nullptr of vertex processor" );
		assert( m_rasterizer.get( ) && "nullptr of rasterizer" );
		assert( m_pixelProcessor.get( ) && "nullptr of pixel processor" );
		assert( m_outputMerger.get( ) && "nullptr of output merger" );

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
		RenderingContext* pRenderingContext = new RenderingContext( 4 /* the number of render targets */ );
		m_renderingContexts.emplace_back( pRenderingContext );
		return m_renderingContexts.at( m_renderingContexts.size() - 1 );
	}
}
