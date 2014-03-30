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

		std::vector<Vector3> m_vertices;
		std::vector<Vector3> m_Color;
		std::map<int, std::vector<int>> m_faces;
		COORDINATE m_coodinate;
	};

	class CPsElementDesc
	{
	public:
		CPsElementDesc( int x, int y, unsigned int color )
			: m_x( x ), m_y( y ), m_color( color ) {}
		~CPsElementDesc( ) {}

		int m_x;
		int m_y;
		//float m_z; //일단은 그려지는 순서대로
		unsigned int m_color;
	};
}

#endif