#include "WPolygon.h"

#include "Utility.h"
#include "WModule.h"
#include "../utility/math3d.h"
#include "../utility/math3d.cpp"

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
	// 미리 계산
	const Matrix4& world = pPainter->GetWorld();
	const Matrix4& view = pPainter->GetView();
	const Matrix4& proj = pPainter->GetProj();

	Matrix4 wvp = world * view * proj;

	int numFace = m_data->GetFaceNum();
	for (int i = 0; i< numFace; ++i)
	{
		unsigned char color[3] = { rand() % 255, rand() % 255, rand() % 255 };
		int numVert = m_data->GetVertexNum(i);

		if (numVert == 0)
		{
			return;
		}

		Vector3 p1 = *m_data->GetVertex(i, numVert - 1);
		pPainter->VertexProcess(wvp, p1);
		for (int vertexIndex = 0; vertexIndex < numVert; ++vertexIndex)
		{
			Vector3 p2 = *m_data->GetVertex(i, vertexIndex);
			pPainter->VertexProcess(wvp, p2);
			InsertLineInfo(pPainter, &p1, &p2, color);
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
	else if ( dy == 0.0f )
	{
		pPainter->InsertLineDepthInfo(Float2Int(v1->Y), Float2Int(v1->X), v1->Z, color);
		pPainter->InsertLineDepthInfo(Float2Int(v2->Y), Float2Int(v2->X), v2->Z, color);
		return;
	}
	else
	{
		gradient = dy / dx;
	}

	// y 절편구하기
	// b = y - ax ( v1 대입 )
	float n = v1->Y - (gradient * v1->X);

	Vector3 start = *v1;
	float endY = roundf(v2->Y);
	float endZ = v2->Z;
	float endX = v2->X;
	
	if (v1->Y > v2->Y)
	{
		start = *v2;
		endY = roundf(v1->Y);
		endZ = v1->Z;
		endX = v1->X;
	}
	
	// Z 보간용 변수
	float lengthZ = v2->Z - v1->Z;
	float lerpZ = start.Z;
	
	// 처음 시작이 부정확해서 시작점은 버텍스 좌표를 찍어준다.
	pPainter->InsertLineDepthInfo(Float2Int(start.Y), Float2Int(start.X), lerpZ, color);

	float y = roundf(start.Y) +1.0f;
	while (y < endY)
	{
		// Z 보간
		float rate = (y - start.Y) / dy;
		lerpZ = start.Z + (lengthZ * rate);

		float x = (y - n) / gradient;
		pPainter->InsertLineDepthInfo(Float2Int(y), Float2Int(x), lerpZ, color);
		y += 1.0f;
	}

	pPainter->InsertLineDepthInfo(Float2Int(endY), Float2Int(endX), endZ, color);
}