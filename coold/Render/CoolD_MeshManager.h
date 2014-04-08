#pragma once
#include "..\Data\CoolD_Type.h"
#include "..\Data\CoolD_Singleton.h"
#include "CoolD_CustomMesh.h"

namespace CoolD
{
	class CustomMesh;
	class MeshManager final : public CSingleton<MeshManager>
	{
		friend class CSingleton<MeshManager>;
	private:
		MeshManager();

	public:
		~MeshManager();	

	public:		
		Dbool LoadMesh( string PathName );
		CustomMesh* GetMesh( string PathName );
		const map<string, CustomMesh*>& GetMeshMap();
		tuple_meshInfo AdjustTransform(CustomMesh* pMesh, const array<Matrix44, TRANSFORM_END>& arrayTransform);
		Duint GetMeshNum() const;
		Dvoid Clear();

		
			

	private:
		CustomMesh* CreateMeshFromFile(const Dchar* filename);		
		
	private:
		map<string, CustomMesh*>	m_mapMesh;		
	};
	
	
};

