#include "WMesh.h"
#include "Utility.h"
#include "WModule.h"

using namespace Utility;
void WMesh::DrawOutline(WModule* pPainter)
{
	int numFace = m_data->GetFaceNum();
	for( int i=0; i<numFace; ++i )
	{
		const unsigned char* color = m_data->GetFaceColor( i );
		int numVert = m_data->GetVertexNum( i );

		if( numVert == 0 )
		{
			return;
		}

		const VERTEX* p1 = m_data->GetVertex(i, numVert -1 );
		for( int vertexIndex = 0; vertexIndex < numVert; ++ vertexIndex )
		{
			const VERTEX* p2 = m_data->GetVertex(i, vertexIndex);
			DrawLine(pPainter, p1, p2, color );
			p1 = p2;
		}
	}
}

void WMesh::DrawLine(WModule* pPainter, const VERTEX* v1, const VERTEX* v2, const unsigned char* rgb)
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
		float startX = 0.0f;
		float endX = 0.0f;
		if( v1->m_pos[0] > v2->m_pos[0])
		{
			startX = v2->m_pos[0];
			endX = v1->m_pos[0];
		}
		else
		{
			startX = v1->m_pos[0];
			endX = v2->m_pos[0];
		}

		float x = startX;
		while( x < endX )
		{
			float y = gradient * x + n;
			pPainter->PaintPixel(Float2Int(x), Float2Int(y), rgb);
			x += 1.0f;
		}
	}
	// y�� �������� ���׸���
	else
	{
		// y = ax + b (a:����, b:y����)
		// ax = y - b
		// x = (y - b) / a
		float startY = 0.0f;
		float endY = 0.0f;
		if( v1->m_pos[1] > v2->m_pos[1])
		{
			startY = v2->m_pos[1];
			endY = v1->m_pos[1];
		}
		else
		{
			startY = v1->m_pos[1];
			endY = v2->m_pos[1];
		}

		float y = startY;
		while( y < endY )
		{
			float x = ( y - n ) / gradient;
			pPainter->PaintPixel( (int)x, (int)y, rgb );
			y += 1.0f;
		}
	}
}

void WMesh::DrawSolid(WModule* pPainter)
{
	int numFace = m_data->GetFaceNum();
	for (int i = 0; i< numFace; ++i)
	{
		const unsigned char* color = m_data->GetFaceColor(i);
		int numVert = m_data->GetVertexNum(i);

		if (numVert == 0)
		{
			return;
		}

		const VERTEX* p1 = m_data->GetVertex(i, numVert - 1);
		for (int vertexIndex = 0; vertexIndex < numVert; ++vertexIndex)
		{
			const VERTEX* p2 = m_data->GetVertex(i, vertexIndex);
			InsertLineInfo(pPainter, p1, p2, color);
			p1 = p2;
		}

		// ������ ��ġ�� �ȼ��� �ѹ��� ĥ�ϵ��� �ϴ� ���� �ʿ�
		pPainter->SortFillInfo();
		pPainter->DrawFillInfo();
		pPainter->ResetFillInfo();
	}
}

void WMesh::InsertLineInfo(WModule* pPainter, const VERTEX* v1, const VERTEX* v2, const unsigned char* rgb)
{
	// ������ ������
	// y = ax + b (a:����, b:y����)

	// �� ���� ������ ���� ���ϱ�
	// b = y - ax

	float dx = v2->m_pos[0] - v1->m_pos[0];
	float dy = v2->m_pos[1] - v1->m_pos[1];

	float gradient = 0.0f;

	if (dx == 0.0f)
	{
		gradient = 1.0f;
	}
	else if (dy == 0.0f)
	{
		return;
	}
	else
	{
		gradient = dy / dx;
	}

	// y �����ϱ�
	// b = y - ax ( v1 ���� )
	float n = v1->m_pos[1] - (gradient * v1->m_pos[0]);

	float startY = v1->m_pos[1];
	float endY = roundf(v2->m_pos[1]);
	if (v1->m_pos[1] > v2->m_pos[1])
	{
		startY = v2->m_pos[1];
		endY = roundf(v1->m_pos[1]);
	}

	float y = roundf(startY);
	while (y < endY)
	{
		float x = (y - n) / gradient;
		pPainter->InsertLineInfo(Float2Int(y), Float2Int(x), rgb);
		y += 1.0f;
	}

}

void WMesh::DrawSolidParallel(WModule* pModule)
{
	UNREFERENCED_PARAMETER(pModule);
}