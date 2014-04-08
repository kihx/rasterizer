#include "CoolD_MeshManager.h"
#include "CoolD_Transform.h"
#include "..\Data\CoolD_Inlines.h"
#include "..\Data\CoolD_Defines.h"


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
	
	tuple_meshInfo MeshManager::AdjustTransform(CustomMesh* mesh, const array<Matrix44, TRANSFORM_END>& arrayTransform)
	{
		vector<Vector3> listVertex;
		if( mesh->GetType() == MSH )
		{
			listVertex = mesh->GetVectorVertex();			
		}
		else if( mesh->GetType() == PLY )
		{			
			for( Duint i = 1; i <= mesh->GetVertexSize(); ++i )
			{					
				listVertex.emplace_back( TransformHelper::TransformVertex(arrayTransform, mesh->GetVertex(i) ));
			}	
		}
		else
		{	//타입지정이 안 되어있음 무조건 실패
			assert(false);
		}
		
		return make_tuple(listVertex, mesh->GetVectorFace() );
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

	const map<string, CustomMesh*>& MeshManager::GetMeshMap()
	{
		return m_mapMesh;
	}	
};

