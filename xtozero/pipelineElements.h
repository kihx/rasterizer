#ifndef _PIPELINEELEMENTS_H_
#define _PIPELINEELEMENTS_H_

#include "XtzMath.h"
#include "Mesh.h"

#include <map>
#include <vector>

namespace xtozero
{
	class CRsElementDesc
	{
	public:
		CRsElementDesc( ) {}
		~CRsElementDesc( ) {}

		std::vector<Vector4> m_vertices;
		std::vector<Vector3> m_Color;
		std::map<int, std::vector<int>> m_faces;
		COORDINATE m_coodinate;
	};

	class CPsElementDesc
	{
	public:
		CPsElementDesc() {}
		CPsElementDesc( int x, int y, float z, unsigned int color )
			: m_x( x ), m_y( y ), m_z( z ), m_color( color ) {}
		~CPsElementDesc( ) {}

		int m_x;
		int m_y;
		float m_z;
		unsigned int m_color;
	};

	class COmElementDesc
	{
	public:
		COmElementDesc() {}
		COmElementDesc( int x, int y, float z, unsigned int color )
			: m_x( x ), m_y( y ), m_z( z ), m_color( color ) {}
		~COmElementDesc( ) {}

		int m_x;
		int m_y;
		float m_z; 
		unsigned int m_color;
	};
}

#endif