#ifndef _PIPELINEELEMENTS_H_
#define _PIPELINEELEMENTS_H_

#include "..//utility/math3d.h"

namespace xtozero
{
	class CRsElementDesc
	{
	public:
		CRsElementDesc( Vector3 vertex, float r, float g, float b )
			: m_Vertex( vertex ), m_Color( r, g, b ) {}
		~CRsElementDesc( ) {}

		Vector3 m_Vertex;
		Vector3 m_Color;
	};

	class CPsElementDesc
	{
	public:
		CPsElementDesc( int x, int y, unsigned int color )
			: m_x( x ), m_y( y ), m_color( color ) {}
		~CPsElementDesc( ) {}

		int m_x;
		int m_y;
		//float m_z; //�ϴ��� �׷����� �������
		unsigned int m_color;
	};
}

#endif