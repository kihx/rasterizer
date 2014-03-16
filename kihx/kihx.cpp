// kihx.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "kihx.h"
#include "mesh.h"



using namespace kihx;


static shared_ptr<Mesh> g_mesh;




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
		VertexProcInputStream::Data outData;

		size_t capacity = 0;
		size_t numFaces = inputStream->NumFaces();
		for ( size_t face = 0; face < numFaces; ++face )
		{
			capacity += inputStream->NumVerticesInFace( face );
		}
		pOutputStream->Reserve( capacity );

		for ( size_t face = 0; face < numFaces; ++face )
		{
			size_t numVertices = inputStream->NumVerticesInFace( face );
			for ( size_t vert = 0; vert < numVertices; ++vert )
			{
				outData.Assign( inputStream->GetVertexInFaceAt( face, vert ) );
				pOutputStream->Push( outData );
			}
		}

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
		RasterizerInputStream::Data outData;

		size_t inputSize = inputStream->Size();
		for ( size_t i = 0; i < inputSize; ++i )
		{
			Transform( inputStream->GetData( i ), outData );
	
			pOutputStream->Push( outData );
		}

		return shared_ptr<RasterizerInputStream>( pOutputStream );
	}

private:
	void Transform( const float vertex[3], RasterizerInputStream::Data& outData )
	{
		// TODO: transform
		outData.X = vertex[0];
		outData.Y = vertex[1];
		outData.Z = vertex[2];
		outData.W = 1.0f;
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

	void Clear( unsigned char r, unsigned char g, unsigned char b, unsigned char a )
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
} rp;


class KihxDevice : public Singleton<KihxDevice>
{
	friend class Singleton<KihxDevice>;

protected:
	KihxDevice()
	{
	};

public:
	virtual ~KihxDevice()
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
	static shared_ptr<RenderingContext> context = KihxDevice::GetInstance()->CreateRenderingContext();
	static shared_ptr<Texture> renderTarget = Texture::Create( width, height, Texture::RGB888, buffer );
	
	context->SetRenderTarget( renderTarget, 0 );

	context->Clear( 222, 180, 25, 255 );

	context->Draw( g_mesh );
}
