#pragma once

#include "base.h"

#include <memory>
#include <vector>


//class IInputOutputStream
//{
//public:
//	//virtual ~IInputOutputStream() = 0;
//};


namespace kih
{
	/* class IRenderingProcessor
	*/
	template<class InputStream, class OutputStream>
	class IRenderingProcessor
	{
	public:
		//virtual ~IRenderingProcessor() = 0;

		virtual std::shared_ptr<OutputStream> Process( std::shared_ptr<InputStream> inputStream ) = 0;

		//virtual IOutputStream* Process( IInputStream* inputStream ) = 0;
		//bool SetInputStream( IInputStream* inputStream );
		//IOutputStream* GetOutputStream();
	};


	class Mesh;

	class ConstantBuffer;
	class Texture;

	class VertexProcInputStream;	
	typedef VertexProcInputStream InputAssemblerOutputStream;

	class RasterizerInputStream;
	typedef RasterizerInputStream VertexProcOutputStream;

	class PixelProcInputStream;
	typedef PixelProcInputStream RasterizerOutputStream;

	class OutputMergerInputStream;
	typedef OutputMergerInputStream PixelProcOutputStream;

	class InputAssembler;
	class VertexProcessor;
	class Rasterizer;
	class PixelProcessor;
	class OutputMerger;
	class RenderingContext;


	enum class ColorFormat : unsigned int
	{
		UNKNOWN = 0,

		RGB888 = 10,
	};

	static int ComputeBytesPerPixel( ColorFormat format )
	{
		switch ( format )
		{
		case ColorFormat::RGB888:
			return 3;

		default:
			return 0;
		}
	}


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
			m_format( ColorFormat::UNKNOWN ),
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

	private:
		bool WriteTexel( int x, int y, byte r, byte g, byte b )
		{
			if ( !HasFlag( FL_LOCKED ) )
			{
				return false;
			}

			assert( ( x >= 0 && x < m_width ) && "out of ranged x-coordinate" );
			assert( ( y >= 0 && y < m_height ) && "out of ranged y-coordinate" );
			assert( ( ComputeBytesPerPixel( Format() ) == 3 ) && "incorrect color format" );

			if ( byte* buffer = static_cast< byte* >( m_pMemory ) )
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
			if ( !HasFlag( FL_LOCKED ) )
			{
				return false;
			}

			assert( ( x >= 0 && x < m_width ) && "out of ranged x-coordinate" );
			assert( ( y >= 0 && y < m_height ) && "out of ranged y-coordinate" );
			assert( ( ComputeBytesPerPixel( Format() ) == 4 ) && "incorrect color format" );

			if ( byte* buffer = static_cast< byte* >( m_pMemory ) )
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

		friend class RenderingContext;
	};

	/* class LockGuard<Texture>
	*/
	template<>
	class LockGuard<Texture>
	{
	public:
		LockGuard( Texture* obj ) :
			m_obj( obj )
		{
			m_obj->Lock( &m_p );
		}

		~LockGuard()
		{
			m_obj->Unlock();
		}

		void* Ptr()
		{
			return m_p;
		}

	private:
		Texture* m_obj;
		void* m_p;
	};


	/* class RenderingContext
	*/
	class RenderingContext
	{
		MAKE_NONCOPYABLE( RenderingContext )

	public:
		explicit RenderingContext( size_t numRenderTargets );

		void Clear( byte r, byte g, byte b, byte a );

		void Draw( std::shared_ptr<Mesh> mesh );

		std::shared_ptr<Texture> GetRenderTaget( size_t index )
		{
			assert( ( index >= 0 && index < m_renderTargets.size() ) && "out of ranged index" );
			return m_renderTargets[index];
		}

		bool SetRenderTarget( std::shared_ptr<Texture> texture, size_t index )
		{
			assert( ( index >= 0 && index < m_renderTargets.size() ) && "out of ranged index" );
			m_renderTargets[index] = texture;
			return true;
		}

	private:
		std::unique_ptr<InputAssembler> m_inputAssembler;
		std::unique_ptr<VertexProcessor> m_vertexProcessor;
		std::unique_ptr<Rasterizer> m_rasterizer;
		std::unique_ptr<PixelProcessor> m_pixelProcessor;
		std::unique_ptr<OutputMerger> m_outputMerger;
		std::vector< std::shared_ptr<Texture> > m_renderTargets;
	};


	/* class RenderingDevice
	*/
	class RenderingDevice : public Singleton<RenderingDevice>
	{
		friend class Singleton<RenderingDevice>;

	private:
		RenderingDevice()
		{
		};

	public:
		~RenderingDevice()
		{
		};

		std::shared_ptr<RenderingContext> CreateRenderingContext();

	private:
		std::vector<std::shared_ptr<RenderingContext>> m_renderingContexts;
	};
};

