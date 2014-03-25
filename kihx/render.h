#pragma once

#include "base.h"

#include <memory>
#include <vector>


namespace kih 
{
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
	typedef OutputMergerInputStream OutputMergerOutputStream;

	class InputAssembler;
	class VertexProcessor;
	class Rasterizer;
	class PixelProcessor;
	class OutputMerger;
	class RenderingContext;


	/* class IRenderingProcessor
	*/
	template<class InputStream, class OutputStream>
	class IRenderingProcessor
	{
	public:
		virtual std::shared_ptr<OutputStream> Process( std::shared_ptr<InputStream> inputStream ) = 0;
	};


	/* class BaseInputOutputStream
	*/
	template<class Data>
	class BaseInputOutputStream
	{
		NONCOPYABLE_CLASS( BaseInputOutputStream );

	public:
		BaseInputOutputStream() = default;
		virtual ~BaseInputOutputStream() = default;

		template <typename... Args>
		void Push( Args&&... args )
		{
			m_streamSource.emplace_back( std::forward<Args>( args )... );
		}

		const Data* GetStreamSource() const
		{
			if ( m_streamSource.empty() )
			{
				return nullptr;
			}
			else
			{
				return &m_streamSource[0];
			}
		}

		const Data& GetData( size_t index ) const
		{
			assert( ( index >= 0 && index < Size() ) && "out of ranged index" );
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

	enum class PrimitiveType : unsigned int
	{
		POINTS = 1,
		LINES = 2,
		TRIANGLES = 3,
		QUADS = 4,
		PENTAGONS = 5,
	};

	inline PrimitiveType GetPrimitiveTypeFromNumberOfVertices( size_t num )
	{
		return static_cast< PrimitiveType >( num );
	}

	inline size_t ComputeNumberOfVerticesPerPrimitive( PrimitiveType type )
	{	
		return static_cast< size_t >( type );
		//switch ( type )
		//{
		//case PrimitiveType::POINTS:
		//	throw std::runtime_error( "unsupported primitive type" );
		//	return 1;

		////case PrimitiveType::LINES:
		////	numVerticesPerPrimitive = 2;
		////	break;

		//case PrimitiveType::TRIANGLES:
		//default:
		//	return 3;
		//}
	}

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


	/* class RenderingContext
	*/
	class RenderingContext final
	{
		NONCOPYABLE_CLASS( RenderingContext )

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
		std::shared_ptr<InputAssembler> m_inputAssembler;
		std::shared_ptr<VertexProcessor> m_vertexProcessor;
		std::shared_ptr<Rasterizer> m_rasterizer;
		std::shared_ptr<PixelProcessor> m_pixelProcessor;
		std::shared_ptr<OutputMerger> m_outputMerger;
		std::vector< std::shared_ptr<Texture> > m_renderTargets;
	};


	/* class RenderingDevice
	*/
	class RenderingDevice final : public Singleton<RenderingDevice>
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

