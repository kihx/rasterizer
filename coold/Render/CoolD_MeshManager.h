#pragma once
#include "..\Data\CoolD_Type.h"
#include "..\Data\CoolD_Singleton.h"
#include "CoolD_CustomMesh.h"
#include "CoolD_FrustumCull.h"

namespace CoolD
{
	class CustomMesh;
	class MeshManager final : public CSingleton<MeshManager>
	{
		friend class CSingleton<MeshManager>;
	private:
		MeshManager() = default;
		MeshManager(const MeshManager&) = delete;
		MeshManager& operator=(const MeshManager&) = delete;

	public:
		~MeshManager();	

	public:		
		Dbool LoadMesh( string PathName );
		CustomMesh* GetMesh( string PathName );
		
		const map<string, CustomMesh*>& GetMapMesh();
		const vector<Vector3>* GetVecTransformVertex();
		Dvoid AdjustTransform(CustomMesh* pMesh, const array<Matrix44, TRANSFORM_END>& arrayTransform);
		Duint GetMeshNum() const;
		Dvoid Clear();				

	private:
		CustomMesh* CreateMeshFromFile(const Dchar* filename);		
		
	private:
		map<string, CustomMesh*>	m_mapMesh;		
		vector<Vector3>		m_trasnformVertex;
		FrustumCull			m_FrustumCull;
	};
	
	
};

