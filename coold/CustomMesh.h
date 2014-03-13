#pragma once

#include <vector>

namespace coold
{
	struct baseVertex
	{
		float x, y, z;
	};

	struct baseFace
	{
		unsigned char		r, g, b, a;
		std::vector<int>	vecIndex;
	};

	class CustomMesh
	{
	private:
		CustomMesh( const char* filename );
	
	public:
		static CustomMesh* CreateMeshFromFile( const char* filename );

	private:
		bool LoadMeshInfo( );

	public:
		const char* m_szFileName;

	private:
		std::vector<baseVertex> m_vecVertex;
		std::vector<baseFace>	m_vecFace;
	};
};