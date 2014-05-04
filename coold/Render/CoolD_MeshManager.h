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
		MeshManager(const MeshManager&) = delete;
		MeshManager& operator=(const MeshManager&) = delete;

	public:
		~MeshManager();	

	public:		
		Dbool LoadMesh( string PathName );
		Dbool AddMesh(string name, CustomMesh* pMesh);
		CustomMesh* GetMesh( string PathName );
		
		const map<string, CustomMesh*>& GetMapMesh();		
		const vector<BaseFace>* GetVecCulledFace();		
		Duint GetMeshNum() const;		
		Dvoid Clear();

	private:
		CustomMesh* CreateMeshFromFile(const Dchar* filename);		
		
	private:
		map<string, CustomMesh*>	m_mapMesh;					
	};	
};

