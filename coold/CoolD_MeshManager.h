#pragma once
#include "CoolD_Type.h"
#include "CoolD_Singleton.h"
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
		const list<CustomMesh*>& AdjustTransform();
		Duint GetMeshNum() const;
		Dvoid Clear();

	private:
		CustomMesh* CreateMeshFromFile(const Dchar* filename);
		CustomMesh* CreateTransformedMesh(CustomMesh* pMesh);
		
	private:
		map<string, CustomMesh*>	m_mapMesh;
		list<CustomMesh*> m_ListMesh;
	};
	
	
};

