#include "stdafx.h"
#include "texture.h"


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

	Texture::Texture() :
		m_width( -1 ),
		m_height( -1 ),
		m_format( ColorFormat::Unknown ),
		m_flags( 0 ),
		m_pMemory( nullptr )
	{
	}

	Texture::~Texture()
	{
	}

	bool Texture::Lock( void** ppMemory )
	{
		if ( HasFlag( FL_LOCKED ) )
		{
			// already locked
			return false;
		}

		if ( m_pMemory == nullptr )
		{
			// not allocated memory
			return false;
		}

		AddFlags( FL_LOCKED );
		*ppMemory = m_pMemory;
		return true;
	}

	void Texture::Unlock()
	{
		RemoveFlags( FL_LOCKED );
	}

	bool Texture::WriteTexel( int x, int y, byte r, byte g, byte b )
	{
		if ( !HasFlag( FL_LOCKED ) )
		{
			return false;
		}

		Assert( ( x >= 0 && x < m_width ) && "out of ranged x-coordinate" );
		Assert( ( y >= 0 && y < m_height ) && "out of ranged y-coordinate" );

		int stride = GetBytesPerPixel( Format() );

		if ( byte* buffer = static_cast< byte* >( m_pMemory ) )
		{
			byte* base = buffer + ( ( ( m_width * y ) + x ) * stride );
			*( base + 0 ) = r;
			*( base + 1 ) = g;
			*( base + 2 ) = b;
			//*( base + 3 ) = a;
		}

		return true;
	}

	void Texture::SetExternalMemory( void* pMemory )
	{
		Assert( pMemory && "nullptr external memory" );

		Purge();

		AddFlags( FL_EXTERNAL_MEMORY );
		m_pMemory = pMemory;
	}

	void Texture::Purge()
	{
		// release internal memory only
		if ( !HasFlag( FL_EXTERNAL_MEMORY ) )
		{
			free( m_pMemory );
		}
		m_pMemory = nullptr;
	}
};
