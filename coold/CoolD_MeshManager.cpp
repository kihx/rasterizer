#include "CoolD_MeshManager.h"
#include "CoolD_Inlines.h"
#include "CoolD_Defines.h"
#include "CoolD_Transform.h"

namespace CoolD
{
	CustomMesh* MeshManager::GetMesh(string PathName)
	{
		return m_mapMesh[ PathName ];
	}

	Dbool MeshManager::LoadMesh(string PathName)
	{
		CustomMesh* pMesh = CreateMeshFromFile(PathName.c_str());

		if( pMesh )
		{
			m_mapMesh.insert(pair<string, CustomMesh*>(PathName.c_str(), pMesh));
			return true;
		}

		return false;		
	}

	Duint MeshManager::GetMeshNum() const
	{
		return m_mapMesh.size();
	}

	const list<CustomMesh*>& MeshManager::AdjustTransform()
	{
		Safe_Delete_VecList( m_ListMesh );		

		for( auto& mesh : m_mapMesh )
		{			
			CustomMesh* transMesh = mesh.second->GetTransformMesh();
			if( transMesh )
			{
				m_ListMesh.push_back( transMesh );
			}
		}
		return m_ListMesh;
	}	 

	MeshManager::MeshManager()
	{
	}

	MeshManager::~MeshManager()
	{		
		Clear();
	}

	CustomMesh* MeshManager::CreateMeshFromFile(const Dchar* filename)
	{
		FixLater( TemplateFactory패턴으로 바꾸자 )
		CustomMesh* pMesh = nullptr;
		if( strstr(filename, ".msh") != nullptr )
		{
			pMesh = new CustomMeshMSH();
			pMesh->Load(filename);
		}
		else if( strstr(filename, ".ply") != nullptr )
		{
			pMesh = new CustomMeshPLY();
			pMesh->Load(filename);
		}

		return pMesh;
	}

	

	Dvoid MeshManager::Clear()
	{
		Safe_Delete_Map(m_mapMesh);
		Safe_Delete_VecList(m_ListMesh);
	}
};

