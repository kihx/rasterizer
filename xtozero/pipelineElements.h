#ifndef _PIPELINEELEMENTS_H_
#define _PIPELINEELEMENTS_H_

#include "..//utility/math3d.h"

class CRsElementDesc
{
public:
	CRsElementDesc(Vector3 vertex, Vector3 color)
		: m_Vertex(vertex), m_Color(color) {}
	~CRsElementDesc() {}

	Vector3 m_Vertex;
	Vector3 m_Color;
};

class CPsElementDesc
{
public:
	CPsElementDesc( int x, int y, unsigned int color )
		: m_x( x ), m_y( y ), m_color( color ) {}
	~CPsElementDesc() {}

	int m_x;
	int m_y;
	//float m_z; //일단은 그려지는 순서대로
	unsigned int m_color;
};

#endif