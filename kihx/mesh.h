#pragma once

#include "base.h"
#include "stdsupport.h"


namespace kih
{
	FORCEINLINE PrimitiveType GetPrimitiveTypeFromNumberOfVertices( size_t num )
	{
		return static_cast< PrimitiveType >( num );
	}

	FORCEINLINE size_t GetNumberOfVerticesPerPrimitive( PrimitiveType type )
	{
		return static_cast< size_t >( type );
	}


	/* Vertex
	*/
	template<class VertexType>
	struct Vertex
	{
		VertexType x;
		VertexType y;
		VertexType z;
	};

	template<class IndexType>
	struct Face
	{
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;
		StlVector<IndexType> m_indices;
	};

	
	/* class VertexBuffer
	*/
	template<class VertexType>
	class VertexBuffer
	{
	public:
		VertexBuffer() = default;
		virtual ~VertexBuffer() = default;

		template <class... Args>
		FORCEINLINE void Push( Args&&... args )
		{ 
			m_vertices.emplace_back( args... );
		}

		FORCEINLINE const Vertex<VertexType>* GetStreamSource() const
		{
			return &m_vertices[0];
		}

		FORCEINLINE const Vertex<VertexType>& GetVertexConst( size_t index ) const
		{
			Assert( ( index >= 0 && index < Size() ) && "out of ranged index" );
			return m_vertices[index];
		}

		FORCEINLINE Vertex<VertexType>& GetVertex( size_t index )
		{
			Assert( ( index >= 0 && index < Size() ) && "out of ranged index" );
			return m_vertices[index];
		}

		FORCEINLINE size_t Size() const
		{
			return m_vertices.size();
		}

		FORCEINLINE void Resize( int size )
		{
			m_vertices.resize( size );
		}
		
		FORCEINLINE void Reserve( int capacity )
		{
			m_vertices.reserve( capacity ); 
		}

	private:
		StlVector<Vertex<VertexType>> m_vertices;
	};

		
	/* class IndexBuffer
	*/
	template<class IndexType>
	class IndexBuffer
	{
	public:
		IndexBuffer() = default;
		virtual ~IndexBuffer() = default;

		template <class... Args>
		FORCEINLINE void Push( Args&&... args )
		{
			m_vertices.emplace_back( args... );
		}

		FORCEINLINE const IndexType* GetStreamSource() const
		{
			return &m_indicies; 
		}

		FORCEINLINE const IndexType& GetIndexConst( size_t index ) const
		{
			Assert( ( index >= 0 && index < Size() ) && "out of ranged index" );
			return m_indicies[index];
		}

		FORCEINLINE IndexType& GetIndex( size_t index )
		{
			Assert( ( index >= 0 && index < Size() ) && "out of ranged index" );
			return m_indicies[index];
		}

		FORCEINLINE size_t Size() const
		{
			return m_indicies.size();
		}

		FORCEINLINE void Resize( int size )
		{
			m_indicies.resize( size );
		}
		
		FORCEINLINE void Reserve( int capacity ) 
		{ 
			m_indicies.reserve( capacity ); 
		}

	private:
		StlVector<IndexType> m_indicies;
	};


	typedef Vertex<float> VertexF;
	typedef Face<unsigned short> FaceS;



	/* class IMesh
	*/
	class IMesh
	{
	public:	
		virtual PrimitiveType GetPrimitiveType() const = 0;
		virtual CoordinatesType GetCoordinatesType() const = 0;

		virtual bool LoadFile( const char* filename ) = 0;
		virtual std::shared_ptr<IMesh> Clone() const = 0;
	};

	// IMesh factory
	std::shared_ptr<IMesh> CreateMeshFromFile( const char* filename );


	/* class IrregularMesh
			It has various number of vertices for each face.
	*/
	class IrregularMesh : public IMesh
	{
		friend std::shared_ptr<IMesh> CreateMeshFromFile( const char* filename );

	private:
		IrregularMesh();
	public:
		virtual ~IrregularMesh();

		virtual PrimitiveType GetPrimitiveType() const;
		virtual CoordinatesType GetCoordinatesType() const;

		virtual bool LoadFile( const char* filename );

		virtual std::shared_ptr<IMesh> Clone() const;

		FORCEINLINE size_t NumFaces() const
		{
			return m_faces.size();
		}

		FORCEINLINE const unsigned char* GetFaceColor( size_t faceIndex ) const
		{
			Assert( ( faceIndex >= 0 && faceIndex < m_faces.size() ) && "out of ranged index" );
			return &m_faces[faceIndex].r;
		}

		FORCEINLINE size_t NumVerticesInFace( size_t faceIndex ) const
		{
			Assert( ( faceIndex >= 0 && faceIndex < m_faces.size() ) && "out of ranged index" );
			return m_faces[faceIndex].m_indices.size();
		}

		FORCEINLINE const float* GetVertexInFaceAt( size_t faceIndex, size_t vertexIndex ) const
		{
			Assert( ( faceIndex >= 0 && faceIndex < m_faces.size() ) && "out of ranged faceIndex" );
			Assert( ( vertexIndex >= 0 && vertexIndex < m_faces[faceIndex].m_indices.size() ) && "out of ranged vertexIndex" );			
			unsigned short index = m_faces[faceIndex].m_indices[vertexIndex] - 1;

			Assert( ( index >= 0 && index < m_vertices.size() ) && "out of ranged index" );
			return &m_vertices[index].x;
		}

	private:
		StlVector<VertexF> m_vertices;
		StlVector<FaceS> m_faces;
	};

	
	/* class OptimizedMesh
			It always has triangulated primitives.
	*/
	class OptimizedMesh : public IMesh
	{
		friend std::shared_ptr<IMesh> CreateMeshFromFile( const char* filename );

	private:
		OptimizedMesh();
	public:
		virtual ~OptimizedMesh();

		virtual PrimitiveType GetPrimitiveType() const;
		virtual CoordinatesType GetCoordinatesType() const;

		virtual bool LoadFile( const char* filename );

		virtual std::shared_ptr<IMesh> Clone() const;

		FORCEINLINE size_t ComputeNumFaces()
		{
			return m_indexBuffer.Size() / 3;
		}

		FORCEINLINE const VertexBuffer<float>& GetVertexBufferConst() const
		{
			return m_vertexBuffer;
		}

		FORCEINLINE VertexBuffer<float>& GetVertexBuffer()
		{
			return m_vertexBuffer;
		}

		FORCEINLINE const IndexBuffer<unsigned short>& GetIndexBufferConst() const 
		{
			return m_indexBuffer;
		}

		FORCEINLINE IndexBuffer<unsigned short>& GetIndexBuffer()
		{
			return m_indexBuffer;
		}

	private:
		VertexBuffer<float> m_vertexBuffer;
		IndexBuffer<unsigned short> m_indexBuffer;
	};
};

using kih::PrimitiveType;
using kih::CoordinatesType;

using kih::IMesh;
using kih::IrregularMesh;
using kih::OptimizedMesh;

