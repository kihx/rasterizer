#pragma once

#include "base.h"
#include <vector>
#include <memory>

#define SUPPORT_MSH		// .msh file format


namespace kih
{
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
	

	template<typename VertexType>
	struct Vertex
	{
		VertexType x;
		VertexType y;
		VertexType z;
	};

	
	/* class VertexBuffer
	*/
	//template<typename VertexType>
	//class VertexBuffer
	//{
	//public:
	//	VertexBuffer() {}

	//	const VertexType* GetStreamSource() const
	//	{
	//		return &m_vertices; 
	//	}
	//	
	//	void Reserve( int capacity )
	//	{
	//		m_vertices.reserve( capacity ); 
	//	}

	//private:
	//	std::vector< Vertex<VertexType> > m_vertices;
	//};

		
	/* class IndexBuffer
	*/
	//template<typename IndexType>
	//class IndexBuffer
	//{
	//public:
	//	IndexBuffer() {}

	//	const IndexType* GetStreamSource() const
	//	{
	//		return &m_indicies; 
	//	}
	//	
	//	void Reserve( int capacity ) 
	//	{ 
	//		m_indicies.reserve( capacity ); 
	//	}

	//private:
	//	std::vector<IndexType> m_indicies;
	//};


	template<typename IndexType>
	struct Face
	{
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;
		std::vector<IndexType> m_indices;
	};


	/* class Mesh
	*/
	class Mesh
	{
		NONCOPYABLE_CLASS( Mesh )

		typedef Vertex<float> VertexF;
		typedef Face<unsigned short> FaceS;

	private:
		Mesh();
	public:
		virtual ~Mesh();

		static std::shared_ptr<Mesh> CreateFromFile( const char* filename );

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
		bool LoadMshFile( const char* filename );
		bool LoadPlyFile( const char* filename );

	private:
		std::vector<VertexF> m_vertices;
		std::vector<FaceS> m_faces;
	};
};

using kih::Mesh;

