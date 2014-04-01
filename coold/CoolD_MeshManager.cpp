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
			CustomMesh* transMesh = CreateTransformedMesh(mesh.second);
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

	CustomMesh* MeshManager::CreateTransformedMesh( CustomMesh* pOriginalMesh)
	{		
		CustomMesh* pMesh = pOriginalMesh->Clone();

		if( pMesh )
		{
			vector<BaseVertex> listVertex;
			if( pMesh->GetMeshType() == MeshType::PLY )
			{				
				for( Duint i = 1; i <= pOriginalMesh->GetVertexSize(); ++i )
				{
					BaseVertex transformVertex(pOriginalMesh->GetVertex(i));

					FixLater(TransformHandler 이건 멀티쓰레드로 가려면 singleton이면 안될듯..)
						listVertex.emplace_back(GETSINGLE(TransformHandler).TransformVertex(transformVertex));
				}

				pMesh->SetVectorVertex(listVertex);
				
			}
			else if( pMesh->GetMeshType() == MeshType::MSH )
			{
				//변환 없음
			}
			return pMesh;
		}
		
		return nullptr;				
	}

	Dvoid MeshManager::Clear()
	{
		Safe_Delete_Map(m_mapMesh);
		Safe_Delete_VecList(m_ListMesh);
	}
};

