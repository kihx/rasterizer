#include "stdafx.h"
#include "Mesh.h"

namespace xtozero
{
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
		std::string filename(pfilename);
		if( m_meshes.find(filename) == m_meshes.end() )
		{
			CMesh* newMesh = new CMesh();
			
			return std::shared_ptr<CMesh>( newMesh );
		}
		else
		{
			//Return Mesh
			return m_meshes[filename];
		}
	}
}