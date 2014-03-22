#include "stdafx.h"
#include "Mesh.h"

namespace xtozero
{
	bool CMesh::LoadFromFile( const char* pfilename )
	{
		CFileHandler meshfile( pfilename );
		if( meshfile.is_open() )
		{
			char token[256] = {0};
			while( meshfile.good() )
			{
				meshfile >> token;

				int symbollen = sizeof("#$") - 1;
				if( strncmp( token, "#$", symbollen ) == 0 )
				{
					if( strncmp( token + symbollen, "Vertices", sizeof("Vertices") ) == 0 )
					{
						int vertices;
						meshfile >> vertices;

						if( vertices <= 0 )
						{
							return false;
						}

						m_vertices.reserve( vertices );
					}
					else if( strncmp( token + symbollen, "Faces", sizeof("Faces") ) == 0 )
					{
						int faces;
						meshfile >> faces;

						if( faces <= 0 )
						{
							return false;
						}

						m_faces.reserve( faces );
					}
				}
				else
				{
					if( strncmp( token, "Vertex", sizeof("Vertex") ) == 0 )
					{
						meshfile >> token;
						Vertex<float> vertex;

						for( int i = 0; i < VETEX_ELEMENT_COUNT; ++i )
						{
							meshfile >> vertex.m_element[i];
						}

						m_vertices.push_back( vertex );
					}
					else if( strncmp( token, "Face", sizeof("Face") ) == 0 )
					{
						meshfile >> token;
						Face face;

						int color;
						for( int i = 0; i < COLOR_ELEMENT_COUNT; ++i )
						{
							meshfile >> color;
							face.m_color[i] = color;
						}

						int indices;
						int index;
						meshfile >> indices;
						for( int i = 0; i < indices; ++i )
						{
							meshfile >> index;
							face.m_indices.push_back( index - 1 );
						}

						m_faces.push_back( face );
					}
				}
			}
			PrintMeshInfo();
			return true;
		}
		else
		{
			return false;
		}
	}

	void CMesh::PrintMeshInfo( void )
	{
		for( auto iter = m_vertices.begin(); iter != m_vertices.end(); ++iter )
		{
			std::cout << "[VERTEX] ";
			for( int i = 0; i < VETEX_ELEMENT_COUNT; ++i )
			{
				std::cout << iter->m_element[i] << " | ";
			}
			std::cout << std::endl;
		}
		for( auto iter = m_faces.begin(); iter != m_faces.end(); ++iter )
		{
			std::cout << "[FACE] " << "color : ";
			for( int i = 0; i < COLOR_ELEMENT_COUNT; ++i )
			{
				std::cout << static_cast<int>(iter->m_color[i]) << " | ";
			}
			std::cout << " indices : ";
			for( auto index = iter->m_indices.begin(); index != iter->m_indices.end(); ++index )
			{
				std::cout << *index << " | ";
			}
			std::cout << std::endl;
		}
	}

	CMeshManager* CMeshManager::m_instance = NULL;

	CMeshManager* CMeshManager::GetInstance( void )
	{
		if( m_instance )
		{
			//Do Nothing
		}
		else
		{
			m_instance = new CMeshManager();
		}

		return m_instance;
	}

	void CMeshManager::ReleaseInstance( void )
	{
		delete m_instance;
		m_instance = NULL;
	}

	std::shared_ptr<CMesh> CMeshManager::LoadMeshFromFile( const char* pfilename )
	{
		if( pfilename == NULL )
		{
			return NULL;
		}

		std::string filename(pfilename);
		std::shared_ptr<CMesh> newMesh;
		if( m_meshes.find(filename) == m_meshes.end() )
		{
			CMesh* pMesh = new CMesh();
			
			if ( pMesh->LoadFromFile(pfilename) )
			{
				m_meshes[filename] = std::shared_ptr<CMesh>( pMesh );
				m_recentMesh = filename;
				return m_meshes[filename];
			}
			else
			{
				delete pMesh;
				return NULL;
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
		if ( m_meshes.find(m_recentMesh) == m_meshes.end() )
		{
			return nullptr;
		}
		else
		{
			return m_meshes[m_recentMesh];
		}
	}
}