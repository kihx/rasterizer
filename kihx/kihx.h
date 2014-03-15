#pragma once

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the KIHX_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// KIHX_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef KIHX_EXPORTS
#define KIHX_API __declspec(dllexport)
#else
#define KIHX_API __declspec(dllimport)
#endif


extern "C" KIHX_API void kiLoadMeshFromFile( const char* filename );

extern "C" KIHX_API void kiRenderToBuffer( void* buffer, int width, int height, int bpp );




#include "base.h"
#include <vector>
#include <memory>


using namespace std;

//class IInputOutputStream
//{
//public:
//	//virtual ~IInputOutputStream() = 0;
//};


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
		m_bpp( -1 ),
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
	static shared_ptr<Texture> Create( int width, int height, int bpp, void* pExternalMemory )
	{
		Texture* pTexture = new Texture();
		pTexture->m_width = width;
		pTexture->m_height = height;
		pTexture->m_bpp = bpp;
		pTexture->m_flags = 0;

		if ( pExternalMemory )
		{
			pTexture->SetExternalMemory( pExternalMemory );
		}
		else
		{
			pTexture->m_pMemory = malloc( width * height * bpp / 8 );
		}
		
		return shared_ptr<Texture>( pTexture );
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

	int BitsPerPixel() const
	{
		return m_bpp;
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
		ppMemory = &m_pMemory;
		return true;
	}

	void Unlock()
	{
		RemoveFlags( EF_LOCKED );
	}

private:
	void WriteTexel( int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a )
	{
		assert( (x >= 0 && x < m_width) && "out of ranged x-coordinate" );
		assert( (y >= 0 && y < m_height) && "out of ranged y-coordinate" );

		if ( unsigned char* buffer = static_cast<unsigned char*>( m_pMemory ) )
		{
			unsigned char* base = buffer + ( ( ( m_width * y ) + x ) * 4 );
			*( base + 0 ) = r;
			*( base + 1 ) = g;
			*( base + 2 ) = b;
			*( base + 3 ) = a;
			//*( buffer + ( ( ( m_width * y ) + x ) * 4 ) + 0 ) = r;
			//*( buffer + ( ( ( m_width * y ) + x ) * 4 ) + 1 ) = g;
			//*( buffer + ( ( ( m_width * y ) + x ) * 4 ) + 2 ) = b;
			//*( buffer + ( ( ( m_width * y ) + x ) * 4 ) + 3 ) = a;
		}
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
	int m_bpp;
	unsigned int m_flags;
	void* m_pMemory;
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
				float X;
				float Y;
				float Z;
			};

			float Value[3];
		};

		Data() :
			X( 0.0f ), Y( 0.0f ), Z( 0.0f )
		{
		}

		Data( const Data& data )
		{
			Assign( data.Value );
		}

		void Assign( const float* value )
		{
			for ( int i = 0; i < 3; ++i )
			{
				Value[i] = value[i];
			}
		}
	};

	VertexProcInputStream()
	{
	}

	void Push( const Data& data )
	{
		m_streamSource.push_back( data );
	}

	const float* GetData( size_t index ) const
	{
		//assert( ( index >=0 && index < Size() ) && "Out of ranged index" );
		return m_streamSource[index].Value;
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

		Data( const Data& data )
		{
			const int DataSize = sizeof( Data );
			memcpy_s( this, DataSize, &data, DataSize );
		}
	};

	RasterizerInputStream() 
	{
	}

	void Push( const Data& data )
	{
		m_streamSource.push_back( data );
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
		//assert( ( index >=0 && index < Size() ) && "Out of ranged index" );
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
