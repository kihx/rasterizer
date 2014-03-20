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
		const baseVertex& GetVertex( Duint index ) const;
		const baseFace&	GetFace(Duint index) const;
		Duint GetVertexSize() const;
		Duint GetFaceSize() const;		

	private:
		bool LoadMeshInfo( );

	public:
		const Dchar* m_szFileName;

	private:
		vector<baseVertex> m_vecVertex;
		vector<baseFace>	m_vecFace;
	};
};