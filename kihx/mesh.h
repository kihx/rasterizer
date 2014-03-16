#pragma once

#include "base.h"
#include <vector>
#include <memory>


namespace kihx
{
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
	//class VertexBuffer : private Uncopyable
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
	//class IndexBuffer : private Uncopyable
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
	class Mesh : private Uncopyable
	{
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

	private:
		std::vector<VertexF> m_vertices;
		std::vector<FaceS> m_faces;
	};
};
