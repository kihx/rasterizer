#pragma once


#include "base.h"


namespace kih
{
	/* enum ColorFormat
	*/
	enum class ColorFormat : unsigned int
	{
		Unknown = 0,

		RGB888 = 10,
	};

	inline int GetBytesPerPixel( ColorFormat format )
	{
		switch ( format )
		{
		case ColorFormat::RGB888:
			return 3;

		default:
			return 0;
		}
	}

	inline ColorFormat GetSuitableColorFormatFromBpp( int bpp )
	{
		switch ( bpp )
		{
		case 24:
			return ColorFormat::RGB888;

		default:
			return ColorFormat::Unknown;
		}
	}


	/* class Texture
	*/
	class Texture
	{
	public:
		// texture flags
		enum
		{
			FL_CLAMP_U = 0x1,
			FL_CLAMP_V = 0x2,

			// if the external memory flag is set, we neither allocate nor deallocate its memory
			FL_EXTERNAL_MEMORY = 0x100,
			// locking to directly access its memory
			FL_LOCKED = 0x1000
		};

	protected:
		Texture() :
			m_width( -1 ),
			m_height( -1 ),
			m_format( ColorFormat::Unknown ),
			m_flags( 0 ),
			m_pMemory( nullptr )
		{
		}

	public:
		virtual ~Texture()
		{
		}

	public:
		// factory
		static std::shared_ptr<Texture> Create( int width, int height, ColorFormat format, void* pExternalMemory );

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
			return ( m_flags & flags ) != 0;
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

		void Unlock()
		{
			RemoveFlags( FL_LOCKED );
		}

		bool WriteTexel( int x, int y, byte r, byte g, byte b )
		{
			if ( !HasFlag( FL_LOCKED ) )
			{
				return false;
			}

			assert( ( x >= 0 && x < m_width ) && "out of ranged x-coordinate" );
			assert( ( y >= 0 && y < m_height ) && "out of ranged y-coordinate" );
			
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

		bool WriteTexel( int x, int y, const byte* color )
		{
			return WriteTexel( x, y, color[0], color[1], color[2] );
		}

		void SetExternalMemory( void* pMemory )
		{
			assert( pMemory && "nullptr external memory" );

			Purge();

			AddFlags( FL_EXTERNAL_MEMORY );
			m_pMemory = pMemory;
		}

		void Purge()
		{
			// release internal memory only
			if ( !HasFlag( FL_EXTERNAL_MEMORY ) )
			{
				free( m_pMemory );
			}
			m_pMemory = nullptr;
		}

	private:
		int m_width;
		int m_height;
		ColorFormat m_format;
		unsigned int m_flags;
		void* m_pMemory;
	};
};

using kih::ColorFormat;
using kih::Texture;
