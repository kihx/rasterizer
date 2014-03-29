#pragma once

#include "base.h"
#include <vector>
#include <memory>

#define SUPPORT_MSH		// .msh file format


namespace kih
{
	/* enum class PrimitiveType
	*/
	enum class PrimitiveType : unsigned int
	{
		Undefined = 0,
		Points = 1,
		Lines = 2,
		Triangles = 3,
		Quads = 4,
		Pentagons = 5,
		Octas = 6
	};

	inline PrimitiveType GetPrimitiveTypeFromNumberOfVertices( size_t num )
	{
		return static_cast< PrimitiveType >( num );
	}

	inline size_t GetNumberOfVerticesPerPrimitive( PrimitiveType type )
	{	
		return static_cast< size_t >( type );
	}


	/* enum class CoordinateType
	*/
	enum class CoordinatesType
	{
		Projective = 0,
		ReciprocalHomogeneous,
	};
	

	/* Vertex
	*/
	template<typename VertexType>
	struct Vertex
	{
		VertexType x;
		VertexType y;
		VertexType z;
	};

	template<typename IndexType>
	struct Face
	{
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;
		std::vector<IndexType> m_indices;
	};

	
	/* class VertexBuffer
	*/
	template<typename VertexType>
	class VertexBuffer
	{
	public:
		VertexBuffer() = default;
		virtual ~VertexBuffer() = default;

		template <typename... Args>
		void Push( Args&&... args )
		{
			m_vertices.emplace_back( args... );
		}

		const Vertex<VertexType>* GetStreamSource() const
		{
			return &m_vertices[0];
		}

		const Vertex<VertexType>& GetVertexConst( size_t index ) const
		{
			assert( ( index >= 0 && index < Size() ) && "out of ranged index" );
			return m_vertices[index];
		}

		Vertex<VertexType>& GetVertex( size_t index )
		{
			assert( ( index >= 0 && index < Size() ) && "out of ranged index" );
			return m_vertices[index];
		}

		size_t Size() const
		{
			return m_vertices.size();
		}

		void Resize( int size )
		{
			m_vertices.resize( size );
		}
		
		void Reserve( int capacity )
		{
			m_vertices.reserve( capacity ); 
		}

	private:
		std::vector<Vertex<VertexType>> m_vertices;
	};

		
	/* class IndexBuffer
	*/
	template<typename IndexType>
	class IndexBuffer
	{
	public:
		IndexBuffer() = default;
		virtual ~IndexBuffer() = default;

		template <typename... Args>
		void Push( Args&&... args )
		{
			m_vertices.emplace_back( args... );
		}

		const IndexType* GetStreamSource() const
		{
			return &m_indicies; 
		}

		const IndexType& GetIndexConst( size_t index ) const
		{
			assert( ( index >= 0 && index < Size() ) && "out of ranged index" );
			return m_indicies[index];
		}

		IndexType& GetIndex( size_t index )
		{
			assert( ( index >= 0 && index < Size() ) && "out of ranged index" );
			return m_indicies[index];
		}

		size_t Size() const
		{
			return m_indicies.size();
		}

		void Resize( int size )
		{
			m_indicies.resize( size );
		}
		
		void Reserve( int capacity ) 
		{ 
			m_indicies.reserve( capacity ); 
		}

	private:
		std::vector<IndexType> m_indicies;
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

		size_t NumFaces() const
		{
			return m_faces.size();
		}

		const unsigned char* GetFaceColor( size_t faceIndex ) const
		{
			assert( ( faceIndex >= 0 && faceIndex < m_faces.size() ) && "out of ranged index" );
			return &m_faces[faceIndex].r;
		}

		size_t NumVerticesInFace( size_t faceIndex ) const
		{
			assert( ( faceIndex >= 0 && faceIndex < m_faces.size() ) && "out of ranged index" );
			return m_faces[faceIndex].m_indices.size();
		}

		const float* GetVertexInFaceAt( size_t faceIndex, size_t vertexIndex ) const
		{
			assert( ( faceIndex >= 0 && faceIndex < m_faces.size() ) && "out of ranged faceIndex" );
			assert( ( vertexIndex >= 0 && vertexIndex < m_faces[faceIndex].m_indices.size() ) && "out of ranged vertexIndex" );			
			unsigned short index = m_faces[faceIndex].m_indices[vertexIndex] - 1;

			assert( ( index >= 0 && index < m_vertices.size() ) && "out of ranged index" );
			return &m_vertices[index].x;
		}

	private:
		std::vector<VertexF> m_vertices;
		std::vector<FaceS> m_faces;
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

		size_t ComputeNumFaces()
		{
			return m_indexBuffer.Size() / 3;
		}

		const VertexBuffer<float>& GetVertexBufferConst() const
		{
			return m_vertexBuffer;
		}

		VertexBuffer<float>& GetVertexBuffer()
		{
			return m_vertexBuffer;
		}

		const IndexBuffer<unsigned short>& GetIndexBufferConst() const 
		{
			return m_indexBuffer;
		}

		IndexBuffer<unsigned short>& GetIndexBuffer()
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

