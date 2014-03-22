#include "stdafx.h"
#include "Rasterizer.h"

namespace xtozero
{
	CRasterizer::CRasterizer(const std::shared_ptr<CMesh> pMesh)
	{
		//여기에서 메시내부의 픽셀을 계산

		for ( int i = 0; i < pMesh->m_faces.size(); ++i )
		{
			CreateEdgeTable(pMesh, i);

			int scanline = m_edgeTable.begin()->first;

			while ( !(m_edgeTable.empty() && m_activeEdgeTable.empty()) )
			{
				UpdateActiveEdgeTable(scanline);

				ProcessScanline();

				++scanline;
			}
		}
	}

	CRasterizer::~CRasterizer(void)
	{

	}

	void CRasterizer::CreateEdgeTable(const std::shared_ptr<CMesh> pMesh, int faceNumber)
	{
		if ( faceNumber >= pMesh->m_faces.size() )
		{
			return;
		}
		//To Do : EdgeTable을 만든다.

		int minY = 0;
		int maxY = 0;
		float minX = 0.0f;
		float gradient = 0.0f;
		float dx = 0.0f;
		float dy = 0.0f;
		Face& face = pMesh->m_faces[faceNumber];

		for ( std::vector<int>::iterator& index = face.m_indices.begin(); index != face.m_indices.end(); ++index )
		{
			if ( (index + 1) == face.m_indices.end() )
			{
				//선분이 성립 안 됨.
				//Do Nothing
				break;
			}
			else
			{
				const Vertex<float>& start = pMesh->m_vertices[*index];
				const Vertex<float>& end = pMesh->m_vertices[*(index + 1)];

				//minY, minX를 구함
				if ( start.m_element[y] > end.m_element[y] )
				{
					maxY = static_cast<int>(start.m_element[y]);
					minY = static_cast<int>(end.m_element[y]);
				}
				else
				{
					maxY = static_cast<int>(end.m_element[y]);
					minY = static_cast<int>(start.m_element[y]);
				}

				minX = min(start.m_element[x], end.m_element[x]);

				//기울기를 구함
				dy = end.m_element[y] - start.m_element[y];
				dx = end.m_element[x] - start.m_element[x];

				if ( dy == 0 )
				{
					gradient = 0.0f;
				}
				else
				{
					gradient = dx / dy;
				}

				//edgeTable에 삽입
				if ( m_edgeTable.find(minY) == m_edgeTable.end() )
				{
					m_edgeTable[minY] = std::vector<Edge>();
				}

				m_edgeTable[minY].emplace_back(minY, maxY, minX, gradient);
			}
		}
	}

	void CRasterizer::UpdateActiveEdgeTable(const int scanline)
	{
		//AET 에서 사용하지 않는 Edge제거
		for ( std::vector<Edge>::iterator& iter = m_activeEdgeTable.begin(); iter != m_activeEdgeTable.end(); )
		{
			if ( iter->m_maxY < scanline )
			{
				iter = m_activeEdgeTable.erase(iter);
			}
			else
			{
				++iter;
			}
		}

		if ( m_edgeTable.empty() )
		{
			//Do Nothing
		}
		else // AET 에 Edge 추가
		{
			std::vector<Edge>& edgelist = m_edgeTable.begin()->second;

			if ( m_edgeTable.begin()->first <= scanline )
			{
				for ( std::vector<Edge>::iterator& edge = edgelist.begin(); edge != edgelist.end(); ++edge )
				{
					m_activeEdgeTable.emplace_back(*edge);
				}
				m_edgeTable.erase(m_edgeTable.begin());
			}

			//minX로 정렬
			std::sort(m_activeEdgeTable.begin(), m_activeEdgeTable.end(),
				[](const Edge& lhs, const Edge& rhs)->bool
			{
				return lhs.m_minX < rhs.m_minX;
			});
		}
	}

	float CRasterizer::GetIntersectXpos( int minY,  int scanlineY, float minX, float gradient) const
	{
		return minX + gradient * (scanlineY - minY);
	}

	void CRasterizer::ProcessScanline()
	{

	}
}