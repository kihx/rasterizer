#pragma once

#include "base.h"
#include "vector.h"

#include <vector>


namespace kih
{
	/* struct VertexProcData
	*/
	struct VertexProcData
	{		
		Vector3 Position;		// object-space position
		//Color32 Color;		// vertex color

		VertexProcData() = default;

		VertexProcData( const float position[3]/*, const Color32& color*/ ) :
			Position( position )/*,
			Color( color )*/
		{
		}

		VertexProcData( const Vector3& position/*, const Color32& color*/ ) :
			Position( position )/*,
			Color( color )*/
		{
		}

		VertexProcData( const VertexProcData& data ) = default;
	};

	/* struct RasterizerData
	*/
	struct RasterizerData
	{		
		Vector4 Position;	// wvp transformed position		
		//Color32 Color;		// vertex color

		RasterizerData() = default;

		RasterizerData( const Vector3& position/*, const Color32& color*/ ) :
			Position( position )/*,
			Color( color )*/
		{
		}

		RasterizerData( const Vector4& position ) :
			Position( position )
		{
		}

		RasterizerData( const RasterizerData& data ) = default;
	};
	
	/* struct PixelProcData
	*/
	struct PixelProcData
	{
		// x and y coordinates of a pixel
		unsigned short PX;
		unsigned short PY;
		
		// z of a pixel
		float Depth;
		
		Color32 Color;		// interpolated color

		PixelProcData() :
			PX( 0 ),
			PY( 0 ),
			Depth( 1.0f ) // farthest
		{
		}

		PixelProcData( unsigned short px, unsigned short py, float Depth, const Color32& color ) :
			PX( px ),
			PY( py ),
			Depth( Depth ),
			Color( color )
		{
		}

		PixelProcData( const PixelProcData& data ) = default;
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

		FORCEINLINE void Reserve( size_t capacity )
		{
			m_streamSource.reserve( capacity );
		}

		FORCEINLINE void Clear()
		{
			m_streamSource.clear();
		}

	private:
		std::vector<Data> m_streamSource;
	};	

	/* class VertexProcInputStream
	*/
	class VertexProcInputStream : public BaseInputOutputStream<VertexProcData>
	{
	public:
		VertexProcInputStream() :
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

	/* class PixelProcInputStream
	*/
	class PixelProcInputStream : public BaseInputOutputStream<PixelProcData>
	{
	public:
		PixelProcInputStream()
		{
		}
	};

	/* class OutputMergerInputStream
	*/
	class OutputMergerInputStream : public BaseInputOutputStream<PixelProcData>
	{
	public:
		OutputMergerInputStream()
		{
		}

	private:

	};
};

