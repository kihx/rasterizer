#ifndef _PIPELINEELEMENTS_H_
#define _PIPELINEELEMENTS_H_

#include "Mesh.h"

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