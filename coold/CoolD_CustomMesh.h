#pragma once

#include "CoolD_Type.h"

namespace CoolD
{
	class CustomMesh
	{
	private:
		CustomMesh( const Dchar* filename );
	public:
		~CustomMesh();
	
	public:
		static CustomMesh* CreateMeshFromFile( const Dchar* filename );
		const BaseVertex& GetVertex( Duint index ) const;
		const BaseFace&	GetFace(Duint index) const;
		Duint GetVertexSize() const;
		Duint GetFaceSize() const;		

	private:
		bool LoadMeshInfo( );

	public:
		const Dchar* m_szFileName;

	private:
		vector<BaseVertex> m_vecVertex;
		vector<BaseFace>	m_vecFace;
	};
};