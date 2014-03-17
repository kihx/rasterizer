#include "WMesh.h"

void WMesh::DrawLine(VERTEX* v1, VERTEX* v2, unsigned char* rgb)
{
	// 직선의 방정식
	// y = ax + b (a:기울기, b:y절편)
	
	// 두 점을 지나는 직선 구하기
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

	// y 절편구하기
	// b = y - ax ( v1 대입 )
	float n = v1->m_pos[1] - ( gradient * v1->m_pos[0] );

	if( abs(dx) > abs(dy))
	{
		// x축 기준으로 선을 그려나가기
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