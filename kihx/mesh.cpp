#include "stdafx.h"
#include "mesh.h"

#include <fstream>


namespace kih
{
	// RAII: Resource Acquisition Is Initialization
	// stream reader class
	class RAIIStreamReader
	{
	public:
		explicit RAIIStreamReader( const char* filename )
		{
			m_reader.open( filename );
		}

		~RAIIStreamReader()
		{
			m_reader.close();
		}

		bool IsOpened() const
		{
			return m_reader.is_open();
		}

		std::ifstream& operator>>( char* value )
		{
			m_reader >> value;
			return m_reader;
		}

		std::ifstream& operator>>( unsigned short& value )
		{
			m_reader >> value;
			return m_reader;
		}

		std::ifstream& operator>>( int& value )
		{
			m_reader >> value;
			return m_reader;
		}

		std::ifstream& operator>>( float& value )
		{
			m_reader >> value;
			return m_reader;
		}

		std::ifstream& operator>>( unsigned char& value )
		{
			// read stream as numeric value and then convert to unsigned char
			int tmp;
			m_reader >> tmp;
			value = (unsigned char) tmp;
			return m_reader;
		}

	private:
		std::ifstream m_reader;
	};


	/* class Mesh
	*/	
	Mesh::Mesh()
	{

	}

	Mesh::~Mesh()
	{

	}

	std::shared_ptr<Mesh> Mesh::CreateFromFile( const char* filename )
	{
		Mesh* pMesh = new Mesh();
		if ( !pMesh->LoadMshFile( filename ) )
		{
			delete pMesh;
			pMesh = nullptr;
		}

		return std::shared_ptr<Mesh>( pMesh );
	}


	/*	sample: input.msh
	#$Vertices       6    # 점의 개수
	#$Faces        2    # 면의 개수
	Vertex         1   0 0 0  # 점번호 x y z
	Vertex         2   639 479 0
	Vertex         3   0 479 0
	Vertex         4   300.5 70.4 0 
	Vertex         5   500.8 50.5 0
	Vertex         6   400.8 450.1 0
	Face 1 0 0 255 3 1 2 3 # 면번호 r g b “점의 개수” “면을 이루는 점들”(점의 개수는 유동적)..
	Face 2 255 0 0 4 4 5 6 1 
	*/
	bool Mesh::LoadMshFile( const char* filename )
	{
		if ( filename == nullptr )
		{
			LOG_WARNING( "nullptr filename" );
			return false;
		}

		RAIIStreamReader reader( filename );
		if ( !reader.IsOpened())
		{
			LOG_WARNING( "File open failure" );
			return false;
		}

		// for tags
		char trash[256];

		// read a header
		reader >> trash;	// #$Vertices
		int vertexCount;
		reader >> vertexCount;
		if ( vertexCount <= 0 )
		{
			LOG_WARNING( "Has no vertex" );
			return false;
		}

		reader >> trash;	// #$Faces
		int faceCount;
		reader >> faceCount;
		if ( faceCount <= 0 )
		{
			LOG_WARNING( "Has no face" );
			return false;
		}

		// vertices
		int vertexNumber;
		m_vertices.resize( vertexCount );
		for ( int i = 0; i < vertexCount; ++i )
		{
			reader >> trash;			// Vertex
			reader >> vertexNumber;	// = index + 1
			reader >> m_vertices[i].x;
			reader >> m_vertices[i].y;
			reader >> m_vertices[i].z;
		}

		// faces
		int faceNumber;
		m_faces.resize( faceCount );
		for ( int i = 0; i < faceCount; ++i )
		{
			reader >> trash;		// Face
			reader >> faceNumber;	// = index + 1
			reader >> m_faces[i].r;
			reader >> m_faces[i].g;
			reader >> m_faces[i].b;
			m_faces[i].a = 255;

			int indexCount;
			reader >> indexCount;
			m_faces[i].m_indices.resize( indexCount );
			for ( int j = 0; j < indexCount; ++j )
			{
				reader >> m_faces[i].m_indices[j];
			}
		}

		return true;
	}

};
