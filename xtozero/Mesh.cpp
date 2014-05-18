#include "stdafx.h"
#include "Mesh.h"
#include "Texture.h"

namespace xtozero
{
	bool CMesh::LoadFromFile( const char* pfilename )
	{
		const char* fileExtension;

		fileExtension = strrchr( pfilename, '.' );

		if ( strncmp( fileExtension, ".ply", strnlen_s( fileExtension, 4 ) ) == 0 )
		{
			return LoadFromPly( pfilename );
		}
		else if ( strncmp( fileExtension, ".msh", strnlen_s( fileExtension, 4 ) ) == 0 )
		{
			return LoadFromMsh( pfilename );
		}
		return false;
	}

	bool CMesh::LoadFromMsh( const char* pfilename )
	{
		CFileHandler meshfile( pfilename, 0 );
		if ( meshfile.is_open( ) )
		{
			char token[256] = { 0 };
			while ( meshfile.good( ) )
			{
				meshfile >> token;

				int symbollen = sizeof("#$") - 1;
				if ( strncmp( token, "#$", symbollen ) == 0 )
				{
					if ( strncmp( token + symbollen, "Vertices", sizeof("Vertices") ) == 0 )
					{
						meshfile >> m_nVerties;

						if ( m_nVerties <= 0 )
						{
							return false;
						}

						m_vertices.reserve( m_nVerties );
					}
					else if ( strncmp( token + symbollen, "Faces", sizeof("Faces") ) == 0 )
					{
						meshfile >> m_nfaces;

						if ( m_nfaces <= 0 )
						{
							return false;
						}

						m_faces.reserve( m_nfaces );
					}
				}
				else
				{
					if ( strncmp( token, "Vertex", sizeof("Vertex") ) == 0 )
					{
						meshfile >> token;
						Vector4 vertex;

						meshfile >> vertex.X >> vertex.Y >> vertex.Z;

						m_vertices.emplace_back( vertex );
					}
					else if ( strncmp( token, "Face", sizeof("Face") ) == 0 )
					{
						meshfile >> token;
						Face face;

						int color;
						for ( int i = 0; i < COLOR_ELEMENT_COUNT; ++i )
						{
							meshfile >> color;
							face.m_color[i] = static_cast<unsigned char>( color );
						}

						int indices;
						int index;
						meshfile >> indices;
						face.m_indices.reserve( indices + 1 );
						for ( int i = 0; i < indices; ++i )
						{
							meshfile >> index;
							face.m_indices.emplace_back( index - 1 );
						}
						face.m_indices.emplace_back( *face.m_indices.begin( ) );//마지막 정점은 시작 정점이다.

						m_faces.emplace_back( face );
					}
				}
			}
			m_coordinate = COORDINATE::WINDOW_COORDINATE;
			return true;
		}
		else
		{
			return false;
		}
	}

	bool CMesh::LoadFromPly( const char* pfilename )
	{
		CFileHandler meshfile( pfilename, 0 );
		if ( meshfile.is_open( ) )
		{
			char token[256] = { 0 };
			while ( meshfile.good( ) )
			{
				meshfile >> token;

				int symbollen = sizeof("#$") - 1;
				if ( strncmp( token, "#$", symbollen ) == 0 )
				{
					if ( strncmp( token + symbollen, "Vertices", sizeof("Vertices") ) == 0 )
					{
						meshfile >> m_nVerties;

						if ( m_nVerties <= 0 )
						{
							return false;
						}

						m_vertices.reserve( m_nVerties );
					}
					else if ( strncmp( token + symbollen, "Faces", sizeof("Faces") ) == 0 )
					{
						meshfile >> m_nfaces;

						if ( m_nfaces <= 0 )
						{
							return false;
						}

						m_faces.reserve( m_nfaces );
					}
				}
				else
				{
					if ( strncmp( token, "Vertex", sizeof("Vertex") ) == 0 )
					{
						meshfile >> token;
						Vector4 vertex;

						meshfile >> vertex.X >> vertex.Y >> vertex.Z;

						m_vertices.emplace_back( vertex );
					}
					else if ( strncmp( token, "Face", sizeof("Face") ) == 0 )
					{
						meshfile >> token;
						Face face;

						//Color 값이 없다.
						//int color;
						for ( int i = 0; i < COLOR_ELEMENT_COUNT; ++i )
						{
							//	meshfile >> color;
							face.m_color[i] = 255;
						}

						int index;
						face.m_indices.reserve( VERTEX_ELEMENT_COUNT + 1 );
						for ( int i = 0; i < VERTEX_ELEMENT_COUNT; ++i )
						{
							meshfile >> index;
							face.m_indices.emplace_back( index - 1 );
						}
						face.m_indices.emplace_back( *face.m_indices.begin( ) );//마지막 정점은 시작 정점이다.

						m_faces.emplace_back( face );
					}
				}
			}
			m_coordinate = COORDINATE::OBJECT_COORDINATE;
			return true;
		}
		else
		{
			return false;
		}
	}

	void CMesh::PrintMeshInfo( void )
	{
		for (std::vector<Vector4>::iterator iter = m_vertices.begin(); iter != m_vertices.end(); ++iter)
		{
			std::cout << "[VERTEX] ";
			for ( int i = 0; i < VERTEX_ELEMENT_COUNT; ++i )
			{
				std::cout << iter->X << " | " << iter->Y << " | " << iter->Z << " | ";
			}
			std::cout << std::endl;
		}
		for ( std::vector<Face>::iterator iter = m_faces.begin(); iter != m_faces.end(); ++iter )
		{
			std::cout << "[FACE] " << "color : ";
			for ( int i = 0; i < COLOR_ELEMENT_COUNT; ++i )
			{
				std::cout << static_cast<int>(iter->m_color[i]) << " | ";
			}
			std::cout << " indices : ";
			for ( std::vector<int>::iterator index = iter->m_indices.begin(); index != iter->m_indices.end(); ++index )
			{
				std::cout << *index << " | ";
			}
			std::cout << std::endl;
		}
	}

	std::shared_ptr<CMesh> CMeshManager::LoadMeshFromFile( const char* pfilename )
	{
		if ( pfilename == nullptr )
		{
			return nullptr;
		}

		std::string filename( pfilename );
		std::shared_ptr<CMesh> newMesh;
		if ( m_meshes.find( filename ) == m_meshes.end() )
		{
			CMesh* pMesh = new CMesh();

			if ( pMesh->LoadFromFile( pfilename ) )
			{
				m_meshes[filename] = std::shared_ptr<CMesh>( pMesh );
				m_recentMesh = filename;
				return m_meshes[filename];
			}
			else
			{
				delete pMesh;
				return nullptr;
			}
		}
		else
		{
			//Return Mesh
			m_recentMesh = filename;
			return m_meshes[filename];
		}
	}

	std::shared_ptr<CMesh> CMeshManager::LoadRecentMesh()
	{
		if ( m_meshes.find( m_recentMesh ) == m_meshes.end() )
		{
			return nullptr;
		}
		else
		{
			return m_meshes[m_recentMesh];
		}
	}
}