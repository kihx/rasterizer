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
	
	Dvoid MeshManager::AdjustTransform(CustomMesh* pMesh, const array<Matrix44, TRANSFORM_END>& arrayTransform)
	{
		//m_FrustumCull.CreateFrustum(arrayTransform[ VIEW ], arrayTransform[ PERSPECTIVE ]);

		m_trasnformVertex.clear();

		if( pMesh->GetType() == MSH )
		{
			m_trasnformVertex = (*pMesh->GetVectorVertex());
		}
		else if( pMesh->GetType() == PLY )
		{
			for( Duint i = 1; i <= pMesh->GetVertexSize(); ++i )
			{
				Vector4 v = TransformHelper::TransformWVP(arrayTransform, pMesh->GetVertex(i));

				//if( m_FrustumCull.CheckFrustumCull(pMesh->GetVertex(i)) )	//포함되는 경우
				//{
				//	
				//}					

				m_trasnformVertex.emplace_back(TransformHelper::TransformViewport(arrayTransform, v));
			}
		}
		else
		{	//타입지정이 안 되어있음 무조건 실패
			assert(false);
		}				
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

		m_trasnformVertex.resize(pMesh->GetVertexSize());

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

	const vector<Vector3>* MeshManager::GetVecTransformVertex()
	{
		return &m_trasnformVertex;
	}

};

