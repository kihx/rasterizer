#include "WContext.h"
#include "WModule.h"

#include "Utility.h"

#include <assert.h>

using namespace Utility;
void WContext::ResetFillInfo()
{
	// 라인정보 비우기
	for (size_t i = m_scanOffset; i <= m_scanCount; ++i)
	{
		m_fillInfo[i].m_edgeData.clear();
	}

	m_scanOffset = m_height;
	m_scanCount = 0;
}

void WContext::MakeLineInfo(const Vector3* v1, const Vector3* v2, const unsigned char* color)
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
		if ((size_t)Float2Int(v1->Y) < m_height && v1->Y > 0)
		{
			InsertLineDepthInfo(Float2Int(v1->Y), Float2Int(v1->X), v1->Z, color);
			InsertLineDepthInfo(Float2Int(v2->Y), Float2Int(v2->X), v2->Z, color);
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
	if (y >= m_height)
	{
		return;
	}
	else if (y >= 0)
	{
		// 처음 시작이 부정확해서 시작점은 버텍스 좌표를 찍어준다.
		InsertLineDepthInfo(Float2Int(start.Y), Float2Int(start.X), lerpZ, color);
		y = roundf(start.Y) +1.0f;
	}
	else
	{
		y = 0.0f;
	}

	if (endY < 0)
	{
		return;
	}

	endY = min(endY, (float)(m_height - 1));
	while (y < endY)
	{
		// Z 보간
		float rate = (y - start.Y) / dy;
		lerpZ = start.Z + (lengthZ * rate);

		float x = (y - n) / gradient;
		InsertLineDepthInfo(Float2Int(y), Float2Int(x), lerpZ, color);
		y += 1.0f;
	}

	InsertLineDepthInfo(Float2Int(endY), Float2Int(endX), endZ, color);
}

void WContext::InsertLineInfo(size_t lineIndex, int posX, const unsigned char* rgb)
{
	assert((lineIndex >= 0 && lineIndex < m_height) && "fillInfo Index out of range");

	// 엣지 정보가 들어있는 공간을 저장
	// offset 부터 count 갯수만큼 그리도록
	if (m_scanOffset > lineIndex)
	{
		m_scanOffset = lineIndex;
	}
	else if (m_scanCount < lineIndex)
	{
		m_scanCount = lineIndex;
	}

	m_fillInfo[lineIndex].Insert(posX, rgb);
}

void WContext::InsertLineDepthInfo(size_t lineIndex, int posX, float depth, const unsigned char* rgb)
{
	assert((lineIndex >= 0 && lineIndex < m_height) && "fillInfo Index out of range");

	// 엣지 정보가 들어있는 공간을 저장
	// offset 부터 count 갯수만큼 그리도록
	if (m_scanOffset > lineIndex)
	{
		m_scanOffset = lineIndex;
	}
	else if (m_scanCount < lineIndex)
	{
		m_scanCount = lineIndex;
	}

	m_fillInfo[lineIndex].Insert(posX, depth, rgb);
}

void WContext::SortFillInfo()
{
	for (size_t i = m_scanOffset; i <= m_scanCount; ++i)
	{
		m_fillInfo[i].Sort();
	}
}

void WContext::DrawFillInfo()
{
	for (size_t i = m_scanOffset; i <= m_scanCount; ++i)
	{
		DrawScanline(i, m_fillInfo[i]);
	}
}

void WContext::DrawScanline(int lineIndex, const EdgeInfo& info)
{
	size_t edgeInfoCount = info.m_edgeData.size();

	// 엣지 정보가 홀수이면 assert 경고
	assert(!(edgeInfoCount & 1) && "Invalid edgeInfoCount.");

	for (size_t i = 0; i + 1 < edgeInfoCount; i += 2)
	{
		const PixelInfo& first = info.m_edgeData[i];
		const PixelInfo& second = info.m_edgeData[i + 1];

		if (first.m_x == second.m_x)
		{
			m_module->ZBufferPaintPixel(first.m_x, lineIndex, first.m_z, first.m_rgb);
			return;
		}

		float dx = (float)(second.m_x - first.m_x);
		float dz = (float)(second.m_z - first.m_z);
		for (int offsetX = first.m_x; offsetX <= second.m_x; ++offsetX)
		{
			float z = first.m_z + dz * ((offsetX - first.m_x) / dx);
			m_module->ZBufferPaintPixel(offsetX, lineIndex, z, first.m_rgb);
		}
	}
}

void WContext::VertexProcess(Matrix4& mat, Vector3& vertex)
{
	vertex.Transform(mat);

	float fHalfWidth = m_width * 0.5f;
	float fHalfHeight = m_height * 0.5f;

	vertex.X = vertex.X * fHalfWidth + fHalfWidth;
	vertex.Y = -vertex.Y * fHalfHeight + fHalfHeight;
}

bool WContext::BackFaceCull(const Vector3& v1, const Vector3& v2, const Vector3& v3, const Matrix4& matWV)
{
	Vector3 point1 = v1 * matWV;
	Vector3 point2 = v2 * matWV;
	Vector3 point3 = v3 * matWV;

	Vector3 edge1 = point1 - point2;
	Vector3 edge2 = point1 - point3;
	Vector3 normal = edge1.CrossProduct(edge2);

	Vector3 viewDir(0, 0, -1);
	if (viewDir.DotProduct(normal) < 0)
	{
		return true;
	}

	return false;
}