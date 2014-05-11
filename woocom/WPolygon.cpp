#include "WPolygon.h"

#include "Utility.h"
#include "WModule.h"
#include "WContext.h"
#include "Math.h"
#include "Thread.h"

extern std::shared_ptr<WThreadPool> g_pool;
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

void WPolygon::DrawSolidParallel(WModule* pModule)
{
	const Matrix4& world = pModule->GetWorld();
	const Matrix4& view = pModule->GetView();
	const Matrix4& proj = pModule->GetProj();

	//Matrix4 wvp = world * view * proj;

	size_t numThread = g_pool->GetThreadNum();
	size_t numFace = m_data->GetFaceNum();
	size_t numPanding = numThread - (numThread % numFace);

	size_t taskCount = (numFace + numPanding ) / numThread;

	size_t offset = 0;
	size_t end = taskCount;

	for (size_t i = 0; i < numThread; ++i)
	{
		g_pool->AddTask([=]()
		{
			WContext* pContext = pModule->GetContext();
			Matrix4 wvp = world * view * proj;
			Matrix4 wv = world * view;

			for (size_t faceIndex = offset; faceIndex < end; ++faceIndex)
			{
				unsigned char color[3] = { rand() % 255, rand() % 255, rand() % 255 };
				size_t numVert = m_data->GetVertexNum(faceIndex);
				if (numVert == 0)
				{
					return;
				}

				// face를 이루는 버텍스는 3개라고 가정
				Vector3 p1 = *m_data->GetVertex(faceIndex, 0);
				Vector3 p2 = *m_data->GetVertex(faceIndex, 1);
				Vector3 p3 = *m_data->GetVertex(faceIndex, 2);

				// backface cull
				if (pContext->BackFaceCull(p1, p2, p3, wv))
				{
					continue;
				}

				pModule->VertexProcess(wvp, p1);
				pModule->VertexProcess(wvp, p2);
				pModule->VertexProcess(wvp, p3);

				pContext->MakeLineInfo(&p1, &p2, color);
				pContext->MakeLineInfo(&p2, &p3, color);
				pContext->MakeLineInfo(&p3, &p1, color);

				// 여러번 겹치는 픽셀을 한번만 칠하도록 하는 것이 필요
				pContext->SortFillInfo();
				pContext->DrawFillInfo();
				pContext->ResetFillInfo();
			}

			pModule->ReturnContext(pContext);
		});

		offset = end;
		end += taskCount;
		end = min(end, numFace);
	}

	g_pool->Join();
}

void WPolygon::DrawLine(WModule* pPainter, const Vector3* v1, const Vector3* v2, const unsigned char* color)
{
	UNREFERENCED_PARAMETER(pPainter);
	UNREFERENCED_PARAMETER(v1);
	UNREFERENCED_PARAMETER(v2);
	UNREFERENCED_PARAMETER(color);
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
		if (Float2Int(v1->Y) < pPainter->GetHeight() && v1->Y > 0)
		{
			pPainter->InsertLineDepthInfo(Float2Int(v1->Y), Float2Int(v1->X), v1->Z, color);
			pPainter->InsertLineDepthInfo(Float2Int(v2->Y), Float2Int(v2->X), v2->Z, color);
		}
		
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
	
	float y = roundf(start.Y);
	if (y >= pPainter->GetHeight())
	{
		return;
	}
	else if (y >= 0)
	{
		// 처음 시작이 부정확해서 시작점은 버텍스 좌표를 찍어준다.
		pPainter->InsertLineDepthInfo(Float2Int(start.Y), Float2Int(start.X), lerpZ, color);
		y = roundf(start.Y) + 1.0f;
	}
	else
	{
		y = 0.0f;
	}

	if (endY < 0)
	{
		return;
	}

	endY = min(endY, (float)(pPainter->GetHeight() - 1));
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