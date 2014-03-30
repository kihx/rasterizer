#include "WPolygon.h"

#include "Utility.h"
#include "WModule.h"
#include "../utility/math3d.h"

using namespace Utility;
void WPolygon::DrawOutline(WModule* pPainter)
{
	int numFace = m_data->GetFaceNum();
	for (int i = 0; i<numFace; ++i)
	{
		unsigned char color[3] = { 255, 0, 0 };
		int numVert = m_data->GetVertexNum(i);

		if (numVert == 0)
		{
			return;
		}

		const Vector3* p1 = m_data->GetVertex(i, numVert - 1);
		for (int vertexIndex = 0; vertexIndex < numVert; ++vertexIndex)
		{
			const Vector3* p2 = m_data->GetVertex(i, vertexIndex);
			DrawLine(pPainter, p1, p2, color);
			p1 = p2;
		}
	}
}

void WPolygon::DrawSolid(WModule* pPainter)
{
	int numFace = m_data->GetFaceNum();
	for (int i = 0; i< numFace; ++i)
	{
		unsigned char color[3] = { 255, 0, 0 };
		int numVert = m_data->GetVertexNum(i);

		if (numVert == 0)
		{
			return;
		}

		const Vector3* p1 = m_data->GetVertex(i, numVert - 1);
		for (int vertexIndex = 0; vertexIndex < numVert; ++vertexIndex)
		{
			const Vector3* p2 = m_data->GetVertex(i, vertexIndex);
			InsertLineInfo(pPainter, p1, p2, color);
			p1 = p2;
		}

		// 여러번 겹치는 픽셀을 한번만 칠하도록 하는 것이 필요
		pPainter->SortFillInfo();
		pPainter->DrawFillInfo();
		pPainter->ResetFillInfo();
	}
}

void WPolygon::DrawLine(WModule* pPainter, const Vector3* v1, const Vector3* v2, const unsigned char* color)
{

}

void WPolygon::InsertLineInfo(WModule* pPainter, const Vector3* v1, const Vector3* v2, const unsigned char* color)
{
	// 직선의 방정식
	// y = ax + b (a:기울기, b:y절편)

	// 두 점을 지나는 직선 구하기
	// b = y - ax

	float dx = v2->X - v1->X;
	float dy = v2->Y - v1->Y;

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

	// y 절편구하기
	// b = y - ax ( v1 대입 )
	float n = v1->Y - (gradient * v1->X);

	float startY = v1->Y;
	float endY = roundf(v2->Y);
	if (v1->Y > v2->Y)
	{
		startY = v2->Y;
		endY = roundf(v1->Y);
	}

	float y = roundf(startY);
	while (y < endY)
	{
		float x = (y - n) / gradient;
		pPainter->InsertLineInfo(Float2Int(y), Float2Int(x), color);
		y += 1.0f;
	}
}