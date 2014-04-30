#pragma once

#include "base.h"
#include "stdsupport.h"
#include "threading.h"
#include "vector.h"


namespace kih
{
	/* struct VertexShaderData
	*/
	struct VertexShaderData
	{		
		Vector3 Position;		// object-space position

		VertexShaderData() = default;

		VertexShaderData( const float position[3] ) :
			Position( position )
		{
		}

		VertexShaderData( const Vector3& position ) :
			Position( position )
		{
		}

		VertexShaderData( const VertexShaderData& data ) = default;
	};

	/* struct RasterizerData
	*/
	struct RasterizerData
	{		
		Vector4 Position;	// wvp transformed position		

		RasterizerData() = default;

		RasterizerData( const Vector3& position ) :
			Position( position )
		{
		}

		RasterizerData( const Vector4& position ) :
			Position( position )
		{
		}

		RasterizerData( const RasterizerData& data ) = default;
	};
	
	/* struct FragmentData
	*/
	struct FragmentData
	{
		// x and y coordinates of a pixel
		unsigned short PX;
		unsigned short PY;
		
		// z of a pixel
		float Depth;

		FragmentData() :
			PX( 0 ),
			PY( 0 ),
			Depth( 1.0f ) // farthest
		{
		}

		FragmentData( unsigned short px, unsigned short py, float Depth ) :
			PX( px ),
			PY( py ),
			Depth( Depth )
		{
		}

		FragmentData( const FragmentData& data ) = default;
	};

	/* struct PixelData
	*/
	struct PixelData
	{
		// x and y coordinates of a pixel
		unsigned short PX;
		unsigned short PY;
		
		// z of a pixel
		float Depth;
		
		Color32 Color;

		PixelData() :
			PX( 0 ),
			PY( 0 ),
			Depth( 1.0f ) // farthest
		{
		}

		PixelData( const FragmentData& fragment, const Color32& color ) :
			PX( fragment.PX ),
			PY( fragment.PY ),
			Depth( fragment.Depth ),
			Color( color )
		{
		}

		PixelData( const PixelData& data ) = default;
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

		template <class... Args>
		FORCEINLINE void Push( Args&&... args )
		{
			m_streamSource.emplace_back( args... );
		}

		FORCEINLINE const Data* GetStreamSource() const
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

		FORCEINLINE const Data& GetDataConst( size_t index ) const
		{
			Assert( ( index >= 0 && index < Size() ) && "out of ranged index" );
			return m_streamSource[index];
		}

		FORCEINLINE Data& GetData( size_t index )
		{
			Assert( ( index >= 0 && index < Size() ) && "out of ranged index" );
			return m_streamSource[index];
		}

		FORCEINLINE size_t Size() const
		{
			return m_streamSource.size();
		}

		FORCEINLINE void Resize( size_t size )
		{
			m_streamSource.resize( size );
		}

		FORCEINLINE size_t Capacity() const
		{
			return m_streamSource.capacity();
		}

		FORCEINLINE void Reserve( size_t capacity )
		{
			m_streamSource.reserve( capacity );
		}

		FORCEINLINE void Clear()
		{
			m_streamSource.clear();
		}

		void MoveFrom( BaseInputOutputStream&& src )
		{
			m_streamSource = std::move( src.m_streamSource );
		}

		void Merge( const BaseInputOutputStream& src )
		{
			Assert( src.Size() > 0 );
			std::copy( src.m_streamSource.begin(), src.m_streamSource.end(), std::back_inserter( m_streamSource ) );
		}
		
	private:
		StlVector<Data> m_streamSource;
	};	

	/* class VertexShaderInputStream
	*/
	class VertexShaderInputStream : public BaseInputOutputStream<VertexShaderData>
	{
	public:
		VertexShaderInputStream() :
			m_coordinatesType( CoordinatesType::Projective )
		{
		}

		FORCEINLINE CoordinatesType GetCoordinatesType() const
		{
			return m_coordinatesType;
		}

		FORCEINLINE void SetCoordinatesType( CoordinatesType coord )
		{
			m_coordinatesType = coord;
		}

	private:
		CoordinatesType m_coordinatesType;
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

		FORCEINLINE CoordinatesType GetCoordinatesType() const
		{
			return m_coordinatesType;
		}

		FORCEINLINE void SetCoordinatesType( CoordinatesType coord )
		{
			m_coordinatesType = coord;
		}

		FORCEINLINE PrimitiveType GetPrimitiveType() const
		{
			return m_primitiveType;
		}

		FORCEINLINE void SetPrimitiveType( PrimitiveType primType )
		{
			m_primitiveType = primType;
		}

	private:
		CoordinatesType m_coordinatesType;
		PrimitiveType m_primitiveType;
	};		

	/* class PixelShaderInputStream
	*/
	class PixelShaderInputStream : public BaseInputOutputStream<FragmentData>
	{
	public:
		PixelShaderInputStream()
		{
		}
	};

	/* class OutputMergerInputStream
	*/
	class OutputMergerInputStream : public BaseInputOutputStream<PixelData>
	{
	public:
		OutputMergerInputStream()
		{
		}

		std::shared_ptr<OutputMergerInputStream> Clone()
		{
			size_t size = Size();
			auto clone = std::make_shared<OutputMergerInputStream>();
			if ( size > 0 )
			{
				clone->Resize( size );
				memcpy( &clone->GetData( 0 ), &GetData( 0 ), sizeof( PixelData ) * size );
			}
			return clone;
		}

	private:

	};

	/* class OutputMergerOutputStream:
		Note that OutputMergerOutputStream is meaningless
		because the output merger directly write data on render targets and a depth-stencil buffer.
		Don't push and handle any data if possible.
	*/
	class OutputMergerOutputStream : public BaseInputOutputStream<PixelData>
	{
	public:
		OutputMergerOutputStream()
		{
		}

	private:

	};


	/* class UnorderedAccessView
	*/
	template<class InputStream>
	class UnorderedAccessView
	{
	public:
		explicit UnorderedAccessView( size_t capacity ) :
			m_inputStream( std::make_shared<InputStream>() )
		{
			m_inputStream->Reserve( capacity );
		}

		~UnorderedAccessView()
		{
		}

		FORCEINLINE Mutex& GetLock()
		{
			return m_streamLock;
		}

		std::shared_ptr<InputStream> GetStreamSource()
		{
			return m_inputStream;
		}

		FORCEINLINE void Reserve( size_t capacity )
		{
			Assert( m_inputStream );

			LockGuard<Mutex> lockGuard( m_streamLock );
			m_inputStream->Reserve( capacity );
		}

		FORCEINLINE void Clear()
		{
			Assert( m_inputStream );

			LockGuard<Mutex> lockGuard( m_streamLock );
			m_inputStream->Clear();
		}

		void Merge( const std::shared_ptr<InputStream>& src )
		{	
			Assert( m_inputStream );

			if ( src == nullptr || src->Size() <= 0 )
			{
				return;
			}

			LockGuard<Mutex> lockGuard( m_streamLock );
			m_inputStream->Merge( *src.get() );
		}

	private:
		std::shared_ptr<InputStream> m_inputStream;
		Mutex m_streamLock;
	};
};

using kih::VertexShaderInputStream;
using kih::RasterizerInputStream;
using kih::PixelShaderInputStream;
using kih::OutputMergerInputStream;
using kih::UnorderedAccessView;
