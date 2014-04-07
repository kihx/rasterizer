#include "stdafx.h"
#include "mesh.h"

#include <fstream>


namespace kih
{
	/* class StreamReader
	*/
	class StreamReader
	{
	public:
		explicit StreamReader( const char* filename )
		{
			m_reader.open( filename );
		}

		~StreamReader()
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
			// Read stream as numeric value and then convert to unsigned char.
			int tmp;
			m_reader >> tmp;
			value = (unsigned char) tmp;
			return m_reader;
		}

	private:
		std::ifstream m_reader;
	};


	// Mesh factory function
	std::shared_ptr<IMesh> CreateMeshFromFile( const char* filename )
	{
		if ( filename == nullptr )
		{
			return nullptr;
		}

		// Extract an extention of the specified file.
		// We assume that the extention's length is three characters.
		size_t length = strlen( filename );
		const char* ext = filename + length - 3;

		std::shared_ptr<IMesh> mesh = nullptr;
		if ( strcmp( ext, "msh" ) == 0 )
		{
			// cannot use std::make_shared<>() by the protected access modifier
			mesh = std::shared_ptr<IrregularMesh>( new IrregularMesh() );
			if ( !mesh->LoadFile( filename ) )
			{
				mesh.reset();
			}			
		}
		else if ( strcmp( ext, "ply" ) == 0 )
		{
			mesh = std::shared_ptr<OptimizedMesh>( new OptimizedMesh() );
			if ( !mesh->LoadFile( filename ) )
			{
				mesh.reset();
			}
		}

		return mesh;
	}



	/* class Mesh
	*/	
	IrregularMesh::IrregularMesh()
	{

	}

	IrregularMesh::~IrregularMesh()
	{

	}	

	PrimitiveType IrregularMesh::GetPrimitiveType() const
	{
		return PrimitiveType::Undefined;
	}

	CoordinatesType IrregularMesh::GetCoordinatesType() const
	{
		return CoordinatesType::ReciprocalHomogeneous;
	}

	bool IrregularMesh::LoadFile( const char* filename )
	{
		if ( filename == nullptr )
		{
			LOG_WARNING( "nullptr filename" );
			return false;
		}

		StreamReader reader( filename );
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

	std::shared_ptr<IMesh> IrregularMesh::Clone() const
	{
		return nullptr;
	}


	/* class OptimizedMesh
	*/
	OptimizedMesh::OptimizedMesh()
	{

	}

	OptimizedMesh::~OptimizedMesh()
	{

	}

	PrimitiveType OptimizedMesh::GetPrimitiveType() const
	{
		return PrimitiveType::Triangles;
	}

	CoordinatesType OptimizedMesh::GetCoordinatesType() const
	{
		return CoordinatesType::Projective;
	}

	bool OptimizedMesh::LoadFile( const char* filename )
	{
		if ( filename == nullptr )
		{
			LOG_WARNING( "nullptr filename" );
			return false;
		}

		StreamReader reader( filename );
		if ( !reader.IsOpened() )
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
		m_vertexBuffer.Resize( vertexCount );
		for ( int i = 0; i < vertexCount; ++i )
		{
			reader >> trash;			// Vertex
			reader >> vertexNumber;	// = index + 1

			Vertex<float>& vertex = m_vertexBuffer.GetVertex( i );
			reader >> vertex.x;
			reader >> vertex.y;
			reader >> vertex.z;
		}

		// indices
		int faceNumber;
		
		// .ply has always three vertices per face.
		const int NumVertices = 3;
		m_indexBuffer.Resize( faceCount * 3 );
		for ( int i = 0; i < faceCount; ++i )
		{
			reader >> trash;		// Face
			reader >> faceNumber;	// = index + 1
			
			for ( int j = 0; j < NumVertices; ++j )
			{
				reader >> m_indexBuffer.GetIndex( i * NumVertices + j );
			}
		}

		return true;
	}	

	std::shared_ptr<IMesh> OptimizedMesh::Clone() const
	{
		auto mesh = std::shared_ptr<OptimizedMesh>( new OptimizedMesh() );

		size_t size = GetVertexBufferConst().Size();
		if ( size > 0 )
		{
			auto& vb = mesh->GetVertexBuffer();
			vb.Resize( size );
			memcpy( &vb.GetVertex( 0 ), &GetVertexBufferConst().GetVertexConst( 0 ), sizeof( Vertex<float> ) * size );
		}

		size = GetIndexBufferConst().Size();
		if ( size > 0 )
		{
			auto& vb = mesh->GetIndexBuffer();
			vb.Resize( size );
			memcpy( &vb.GetIndex( 0 ), &GetIndexBufferConst().GetIndexConst( 0 ), sizeof( unsigned short ) * size );
		}

		return mesh;
	}
};
