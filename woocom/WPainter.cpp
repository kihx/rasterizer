#include "WPainter.h"
#include "WModule.h"

#include <assert.h>

void WPainter::ResetFillInfo()
{
	// 라인정보 비우기
	size_t num = m_fillInfo.size();
	for (size_t i = 0; i < num; ++i)
	{
		m_fillInfo[i].m_edgeData.clear();
	}
}

void WPainter::InsertLineInfo(int lineIndex, int posX, const unsigned char* rgb)
{
	assert((lineIndex >= 0 && lineIndex < m_height) && "fillInfo Index out of range");

	m_fillInfo[lineIndex].Insert(posX, rgb);
}

void WPainter::InsertLineDepthInfo(int lineIndex, int posX, float depth, const unsigned char* rgb)
{
	assert((lineIndex >= 0 && lineIndex < m_height) && "fillInfo Index out of range");

	m_fillInfo[lineIndex].Insert(posX, depth, rgb);
}

void WPainter::SortFillInfo()
{
	size_t num = m_fillInfo.size();
	for (size_t i = 0; i < num; ++i)
	{
		m_fillInfo[i].Sort();
	}
}

void WPainter::DrawFillInfo()
{
	size_t num = m_fillInfo.size();
	for (size_t i = 0; i < num; ++i)
	{
		DrawScanline(i, m_fillInfo[i]);
	}
}

void WPainter::DrawScanline(int lineIndex, const EdgeInfo& info)
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