#include "WMesh.h"

void WMesh::DrawLine(VERTEX* v1, VERTEX* v2, unsigned char* rgb)
{
	// ������ ������
	// y = ax + b (a:����, b:y����)
	
	// �� ���� ������ ���� ���ϱ�
	// b = y - ax

	float dx = v2->m_pos[0] - v1->m_pos[0];
	float dy = v2->m_pos[1] - v1->m_pos[1];
	
	float gradient = 0.0f;

	if( dx == 0.0f)
	{
		gradient = 1.0f;
	}
	else if( dy == 0.0f )
	{
		gradient = 0.0f;
	}
	else
	{
		gradient = dy / dx;
	}

	// y �����ϱ�
	// b = y - ax ( v1 ���� )
	float n = v1->m_pos[1] - ( gradient * v1->m_pos[0] );

	if( abs(dx) > abs(dy))
	{
		// x�� �������� ���� �׷�������
		int startX = 0;
		int endX = 0;
		if( v1->m_pos[0] > v2->m_pos[0])
		{
			startX = (int)v2->m_pos[0];
			endX = (int)v1->m_pos[0];
		}
		else
		{
			startX = (int)v1->m_pos[0];
			endX = (int)v2->m_pos[0];
		}

		for(int x=startX; x < endX; ++x )
		{

		}
	}


}