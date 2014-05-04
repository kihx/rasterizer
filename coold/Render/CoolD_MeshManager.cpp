#include "CoolD_MeshManager.h"
#include "CoolD_Transform.h"
#include "..\Data\CoolD_Inlines.h"
#include "..\Data\CoolD_Defines.h"
#include "..\Console\CoolD_Command.h"

namespace CoolD
{
	MeshManager::MeshManager()
	{		
	}

	CustomMesh* MeshManager::GetMesh(string PathName)
	{
		return m_mapMesh[ PathName ];
	}
		
	Dbool MeshManager::LoadMesh(string PathName)
	{
		CustomMesh* pMesh = CreateMeshFromFile(PathName.c_str());

		return AddMesh(PathName, pMesh);
	}

	Dbool MeshManager::AddMesh(string name, CustomMesh* pMesh)
	{
		if (pMesh)
		{
			m_mapMesh.insert(pair<string, CustomMesh*>( name.c_str(), pMesh ));
			return true;
		}
		return false;
	}

	Duint MeshManager::GetMeshNum() const
	{
		return m_mapMesh.size();
	}	

	MeshManager::~MeshManager()
	{		
		Clear();
	}

	CustomMesh* MeshManager::CreateMeshFromFile(const Dchar* filename)
	{	
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
	}

	const map<string, CustomMesh*>& MeshManager::GetMapMesh()
	{
		return m_mapMesh;
	}		
};

