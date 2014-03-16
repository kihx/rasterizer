// kihx.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "kihx.h"
#include "base.h"
#include "mesh.h"

#include <vector>
#include <memory>


using namespace kihx;
using namespace std;

//class IInputOutputStream
//{
//public:
//	//virtual ~IInputOutputStream() = 0;
//};


namespace kihx
{
	template<class InputStream, class OutputStream>
	class IRenderingProcessor
	{
	public:
		//virtual ~IRenderingProcessor() = 0;

		virtual shared_ptr<OutputStream> Process( shared_ptr<InputStream> inputStream ) = 0;

		//virtual IOutputStream* Process( IInputStream* inputStream ) = 0;
		//bool SetInputStream( IInputStream* inputStream );
		//IOutputStream* GetOutputStream();
	};


	class ConstantBuffer;
	class Texture;

	class VertexProcInputStream;
	class RasterizerInputStream;
	class PixelProcInputStream;
	class OutputMergerInputStream;

	class InputAssembler;
	class VertexProcessor;
	class Rasterizer;
	class PixelProcessor;
	class OutputMerger;
	class RenderingContext;



	class Texture
	{
	public:
		enum ColorFormat
		{
			UNKNOWN = 0,

			RGB888 = 10,
		};

		enum
		{
			EF_CLAMP_U = 0x1,
			EF_CLAMP_V = 0x2,

			// if the external memory flag is set, we neither allocate nor deallocate its memory
			EF_EXTERNAL_MEMORY = 0x100,
			// locking to directly access its memory
			EF_LOCKED = 0x1000
		};

	protected:
		Texture() :
			m_width( -1 ),
			m_height( -1 ),
			m_format( UNKNOWN ),
			m_flags( 0 ),
			m_pMemory( NULL )
		{
		}

	public:
		virtual ~Texture()
		{
		}

	public:
		// factory
		static shared_ptr<Texture> Create( int width, int height, ColorFormat format, void* pExternalMemory )
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

			return shared_ptr<Texture>( pTexture );
		}

		static int ToBytesPerPixel( ColorFormat format )
		{
			switch ( format )
			{
			case RGB888:
				return 3;

			default:
				return 0;
			}
		}

	public:
		int Width() const
		{
			return m_width;
		}

		int Height() const
		{
			return m_height;
		}

		ColorFormat Format() const
		{
			return m_format;
		}

		bool HasFlag( unsigned int flags )
		{
			return (m_flags & flags) != 0;
		}

		void AddFlags( unsigned int flags )
		{
			m_flags |= flags;
		}

		void RemoveFlags( unsigned int flags )
		{
			m_flags &= ~flags;
		}

		bool Lock( void** ppMemory )
		{
			if ( HasFlag( EF_LOCKED ) )
			{
				// already locked
				return false;
			}

			if ( m_pMemory == NULL )
			{
				// not allocated memory
				return false;
			}

			AddFlags( EF_LOCKED );
			*ppMemory = m_pMemory;
			return true;
		}

		void Unlock()
		{
			RemoveFlags( EF_LOCKED );
		}

	private:
		bool WriteTexel( int x, int y, byte r, byte g, byte b )
		{
			if ( !HasFlag( EF_LOCKED ) )
			{
				return false;
			}

			assert( (x >= 0 && x < m_width) && "out of ranged x-coordinate" );
			assert( (y >= 0 && y < m_height) && "out of ranged y-coordinate" );
			assert( (ToBytesPerPixel( Format() ) == 3) && "incorrect color format" );

			if ( byte* buffer = static_cast<byte*>( m_pMemory ) )
			{
				byte* base = buffer + ( ( ( m_width * y ) + x ) * 3 );			
				*( base + 0 ) = r;
				*( base + 1 ) = g;
				*( base + 2 ) = b;
			}

			return true;
		}

		bool WriteTexel( int x, int y, byte r, byte g, byte b, byte a )
		{
			if ( !HasFlag( EF_LOCKED ) )
			{
				return false;
			}

			assert( (x >= 0 && x < m_width) && "out of ranged x-coordinate" );
			assert( (y >= 0 && y < m_height) && "out of ranged y-coordinate" );
			assert( (ToBytesPerPixel( Format() ) == 4) && "incorrect color format" );

			if ( byte* buffer = static_cast<byte*>( m_pMemory ) )
			{
				byte* base = buffer + ( ( ( m_width * y ) + x ) * 4 );
				*( base + 0 ) = r;
				*( base + 1 ) = g;
				*( base + 2 ) = b;
				*( base + 3 ) = a;
			}

			return true;
		}

		void SetExternalMemory( void* pMemory )
		{
			assert( pMemory && "null external memory" );

			Purge();

			AddFlags( EF_EXTERNAL_MEMORY );
			m_pMemory = pMemory;
		}

		void Purge() 
		{
			// release internal memory only
			if ( !HasFlag( EF_EXTERNAL_MEMORY) )
			{
				free( m_pMemory );
			}
			m_pMemory = NULL;
		}

	private:
		int m_width;
		int m_height;
		ColorFormat m_format;
		unsigned int m_flags;
		void* m_pMemory;

		friend class RenderingContext;
	};


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
 				X( 0.0f ), Y( 0.0f ), Z( 0.0f ),
 				R( 0 ), G( 0 ), B( 0 ), A( 0 )
			{
			}

			Data( const float position[3], const byte color[4] ) :
				X( position[0] ), Y( position[1] ), Z( position[2] ),
				R( color[0] ), G( color[1] ), B( color[2] )
			{
			}

			Data( const Data& data )
			{
				Assign( data.Position, data.Color );
			}

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

		void Push( const float position[3], const byte color[4] )
		{
			m_streamSource.emplace_back( position, color );
		}

		const float* GetData( size_t index ) const
		{
			assert( ( index >=0 && index < Size() ) && "Out of ranged index" );
			return m_streamSource[index].Position;
		}

		const float* GetStreamSource() const
		{
			if ( m_streamSource.empty() )
			{
				return NULL;
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

				float Value[4];
			};

			Data() :
				X( 0.0f ), Y( 0.0f ), Z( 0.0f ), W( 1.0f )
			{
			}

			Data( const float position[4] ) :
				X( position[0] ), Y( position[1] ), Z( position[2] ), W( position[3] )
			{
			}

			Data( const Data& data )
			{
				const int DataSize = sizeof( Data );
				memcpy_s( this, DataSize, &data, DataSize );
			}
		};

		RasterizerInputStream() 
		{
		}

		void Push( const float position[4] )
		{
			m_streamSource.emplace_back( position );
		}

		const float* GetStreamSource() const
		{
			if ( m_streamSource.empty() )
			{
				return NULL;
			}
			else
			{
				return &m_streamSource[0].X; 
			}
		}

		const float* GetData( size_t index ) const
		{
			assert( ( index >=0 && index < Size() ) && "Out of ranged index" );
			return m_streamSource[index].Value;
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

	class PixelProcInputStream
	{
	public:
		PixelProcInputStream()
		{
		}
	};

	class OutputMergerInputStream
	{
	public:
		OutputMergerInputStream()
		{
		}

	private:
		void* m_pRenderBuffer;
	};




	class InputAssembler : private Uncopyable, public IRenderingProcessor<Mesh, VertexProcInputStream>
	{
	public:
		explicit InputAssembler( RenderingContext* pRenderingContext ) :
			m_pRenderingContext( pRenderingContext )
		{
		}

		virtual ~InputAssembler()
		{
		}

		virtual shared_ptr<VertexProcInputStream> Process( shared_ptr<Mesh> inputStream )
		{	
			VertexProcInputStream* pOutputStream = new VertexProcInputStream();			

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
					pOutputStream->Push( inputStream->GetVertexInFaceAt( face, vert ), color );
				}
			}
#endif

			return shared_ptr<VertexProcInputStream>( pOutputStream );
		}

	private:
		RenderingContext* m_pRenderingContext;
	};


	class VertexProcessor : private Uncopyable, public IRenderingProcessor<VertexProcInputStream, RasterizerInputStream>
	{
	public:
		explicit VertexProcessor( RenderingContext* pRenderingContext ) :
			m_pRenderingContext( pRenderingContext )
		{
		}

		virtual ~VertexProcessor()
		{
		}

		virtual shared_ptr<RasterizerInputStream> Process( shared_ptr<VertexProcInputStream> inputStream )
		{	
			RasterizerInputStream* pOutputStream = new RasterizerInputStream();		

			size_t inputSize = inputStream->Size();
			for ( size_t i = 0; i < inputSize; ++i )
			{
				float transformedPosition[4];
				Transform( inputStream->GetData( i ), transformedPosition );
	
				pOutputStream->Push( transformedPosition );
			}

			return shared_ptr<RasterizerInputStream>( pOutputStream );
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
		//	return NULL;
		//}

	private:
		RenderingContext* m_pRenderingContext;
	};

	class Rasterizer : private Uncopyable, public IRenderingProcessor<RasterizerInputStream, PixelProcInputStream>
	{
	public:
		explicit Rasterizer( RenderingContext* pRenderingContext ) :
			m_pRenderingContext( pRenderingContext )
		{
		}

		virtual ~Rasterizer()
		{
		}

		virtual shared_ptr<PixelProcInputStream> Process( shared_ptr<RasterizerInputStream> inputStream )
		{	
			PixelProcInputStream* pOutputStream = new PixelProcInputStream();

		
			return shared_ptr<PixelProcInputStream>( pOutputStream );
		}

	private:
		RenderingContext* m_pRenderingContext;
	};

	class PixelProcessor : private Uncopyable, public IRenderingProcessor<PixelProcInputStream, OutputMergerInputStream>
	{
	public:
		explicit PixelProcessor( RenderingContext* pRenderingContext ) :
			m_pRenderingContext( pRenderingContext )
		{
		}

		virtual ~PixelProcessor()
		{
		}

		virtual shared_ptr<OutputMergerInputStream> Process( shared_ptr<PixelProcInputStream> inputStream )
		{	
			OutputMergerInputStream* pOutputStream = new OutputMergerInputStream();
			
			return shared_ptr<OutputMergerInputStream>( pOutputStream );
		}

	private:
		RenderingContext* m_pRenderingContext;
	};

	class OutputMerger : private Uncopyable, public IRenderingProcessor<OutputMergerInputStream, OutputMergerInputStream>
	{
	public:
		explicit OutputMerger( RenderingContext* pRenderingContext ) :
			m_pRenderingContext( pRenderingContext )
		{
		}

		virtual ~OutputMerger()
		{
		}

		virtual shared_ptr<OutputMergerInputStream> Process( shared_ptr<OutputMergerInputStream> inputStream )
		{	
			//OutputMergerInputStream* pOut = new OutputMergerInputStream();

			//inputStream.reset();
			//return shared_ptr<OutputMergerInputStream>( pOut );
			return inputStream;
		}

	private:
		RenderingContext* m_pRenderingContext;
	};


	class RenderingContext : private Uncopyable
	{
	public:
		RenderingContext() :
			m_inputAssembler( this ),
			m_vertexProcessor( this ),
			m_rasterizer( this ),
			m_pixelProcessor( this ),
			m_outputMerger( this )
		{
			__TODO( dynamically set the number of render targets )
			m_renderTargets.resize( 4 );
		}

		void Clear( byte r, byte g, byte b, byte a )
		{
			//unsigned int color = ( r | ( ((unsigned short) g) << 8 ) | ( ((unsigned int) b) << 16 )) | ( ((unsigned int) a) << 24 );

			size_t num = m_renderTargets.size();
			for ( size_t i = 0; i < num; ++i )
			{
				if ( Texture* pTexture = m_renderTargets[i].get() )
				{
					void* p = NULL;
					if ( pTexture->Lock( &p ) )
					{
						SafeUnlock<Texture> unlock( pTexture );

						assert( p && "a null pointer of internal memory of a texture" );

						if ( r == g && g == b )
						{
							int size = pTexture->Width() * pTexture->Height() * Texture::ToBytesPerPixel( pTexture->Format() );
							memset( p, r, size );
						}
						else
						{
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
		}

		void Draw( shared_ptr<Mesh> mesh )
		{
			// input assembler
			shared_ptr<VertexProcInputStream> vpInput = m_inputAssembler.Process( mesh );

			// vertex processor
			shared_ptr<RasterizerInputStream> raInput = m_vertexProcessor.Process( vpInput );

			// rasterizer
			shared_ptr<PixelProcInputStream> ppInput = m_rasterizer.Process( raInput );

			// pixel processor
			shared_ptr<OutputMergerInputStream> omInput = m_pixelProcessor.Process( ppInput );

			// output merger
			omInput = m_outputMerger.Process( omInput );
		}

		shared_ptr<Texture> GetRenderTaget( size_t index )
		{
			assert( (index >= 0 && index < m_renderTargets.size()) && "out of ranged index" );
			return m_renderTargets[index];
		}

		bool SetRenderTarget( shared_ptr<Texture> texture, size_t index )
		{
			assert( (index >= 0 && index < m_renderTargets.size()) && "out of ranged index" );
			m_renderTargets[index] = texture;
			return true;
		}

	private:
		InputAssembler m_inputAssembler;
		VertexProcessor m_vertexProcessor;
		Rasterizer m_rasterizer;
		PixelProcessor m_pixelProcessor;
		OutputMerger m_outputMerger;
		vector< shared_ptr<Texture> > m_renderTargets;
	};


	class RenderingDevice : public Singleton<RenderingDevice>
	{
		friend class Singleton<RenderingDevice>;

	private:
		RenderingDevice()
		{
		};

	public:
		/*virtual*/ ~RenderingDevice()
		{
		};

		shared_ptr<RenderingContext> CreateRenderingContext()
		{
			RenderingContext* pRenderingContext = new RenderingContext();
			m_renderingContexts.push_back( shared_ptr<RenderingContext>( pRenderingContext ) );
			return m_renderingContexts.at( m_renderingContexts.size() - 1 );
		}

	private:
		vector< shared_ptr<RenderingContext> > m_renderingContexts;
	};


	struct EdgeElement
	{
		unsigned short YMax;
		unsigned short XMin;
		float Slope;
	};

	class ScanlinePainter : private Uncopyable
	{
	public:
		ScanlinePainter()
		{
		}

		~ScanlinePainter()
		{
		}

		bool BuildEdgeTable(  )
		{
		}

	private:
		std::vector< std::vector<EdgeElement> > m_edgeTable;
	};
};




static shared_ptr<Mesh> g_mesh;


KIHX_API void kiLoadMeshFromFile( const char* filename )
{
	g_mesh = Mesh::CreateFromFile( filename );
}

KIHX_API void kiRenderToBuffer( void* buffer, int width, int height, int bpp )
{
	if ( buffer == NULL )
	{
		return;
	}

	// clear buffer
	size_t bufferSize = width * height * (bpp / 8);		
	::memset( buffer, 255, bufferSize );


	__UNDONE( temporal testing code );
	static shared_ptr<RenderingContext> context = RenderingDevice::GetInstance()->CreateRenderingContext();
	static shared_ptr<Texture> renderTarget = Texture::Create( width, height, Texture::RGB888, buffer );
	
	context->SetRenderTarget( renderTarget, 0 );

	context->Clear( 222, 180, 25, 255 );

	context->Draw( g_mesh );
}
