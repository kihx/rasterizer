#pragma once

#include "base.h"


namespace kih
{
	FORCEINLINE int GetBytesPerPixel( ColorFormat format )
	{
		switch ( format )
		{
		case ColorFormat::R8G8B8:
			return 3;

		case ColorFormat::D8S24:
		case ColorFormat::D32F:
			return 4;

		default:
			Assert( 0 && "invalid operation" );
			return 0;
		}
	}

	FORCEINLINE ColorFormat GetSuitableColorFormatFromBpp( int bpp )
	{
		switch ( bpp )
		{
		case 24:
			return ColorFormat::R8G8B8;

		default:
			Assert( 0 && "invalid operation" );
			return ColorFormat::Unknown;
		}
	}

	FORCEINLINE ColorFormat GetSuitableDepthStencilFormatFromBpp( int bpp )
	{
		switch ( bpp )
		{
		case 32:
			Assert( 0 && "ambiguous format" );
			return ColorFormat::D8S24;
			//return ColorFormat::D32F;

		default:
			Assert( 0 && "invalid operation" );
			return ColorFormat::Unknown;
		}
	}

	FORCEINLINE bool ColorFormat_IsColor( ColorFormat format )
	{
		unsigned int e = static_cast<unsigned int>( format );
		return ( e >= static_cast< unsigned int >( ColorFormat::R8G8B8 ) &&
			e < static_cast< unsigned int >( ColorFormat::D8S24 ) );
	}

	FORCEINLINE bool ColorFormat_IsDepthStencil( ColorFormat format )
	{
		return !ColorFormat_IsColor( format );
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
		Texture();
	public:
		virtual ~Texture();

	public:
		// factory
		static std::shared_ptr<Texture> Create( int width, int height, ColorFormat format, void* pExternalMemory );

	public:
		FORCEINLINE int Width() const
		{
			return m_width;
		}

		FORCEINLINE int Height() const
		{
			return m_height;
		}

		FORCEINLINE ColorFormat Format() const
		{
			return m_format;
		}

		FORCEINLINE bool HasFlag( unsigned int flags )
		{
			return ( m_flags & flags ) != 0;
		}

		FORCEINLINE void AddFlags( unsigned int flags )
		{
			m_flags |= flags;
		}

		FORCEINLINE void RemoveFlags( unsigned int flags )
		{
			m_flags &= ~flags;
		}

		bool Lock( void** ppMemory );
		void Unlock();

		bool WriteTexel( int x, int y, byte r, byte g, byte b );

		void SetExternalMemory( void* pMemory );

		void Purge();

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
