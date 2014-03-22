#include "stdafx.h"
#include "Rasterizer.h"

namespace xtozero
{
	CRasterizer::CRasterizer(const std::shared_ptr<CMesh> pMesh)
	{
		//���⿡�� �޽ó����� �ȼ��� ���
		CreateEdgeTable(pMesh);

		int scanline = m_edgeTable.begin()->first;

		while ( !( m_edgeTable.empty() && m_activeEdgeTable.empty() ) )
		{
			UpdateActiveEdgeTable(scanline);

			++scanline;
		}
	}

	CRasterizer::~CRasterizer(void)
	{

	}

	void CRasterizer::CreateEdgeTable(const std::shared_ptr<CMesh>& pMesh)
	{
		//To Do : EdgeTable�� �����.

		int minY = 0;
		int maxY = 0;
		float minX = 0.0f;
		float gradient = 0.0f;
		float dx = 0.0f;
		float dy = 0.0f;

		for ( auto iter = pMesh->m_faces.begin(); iter != pMesh->m_faces.end(); ++iter )
		{
			for ( std::vector<int>::iterator index = iter->m_indices.begin(); index != iter->m_indices.end(); ++index )
			{
				if ( ( index + 1 )  == iter->m_indices.end() )
				{
					//������ ���� �� ��.
					//Do Nothing
					break;
				}
				else
				{
					const Vertex<float>& start = pMesh->m_vertices[*index];
					const Vertex<float>& end = pMesh->m_vertices[*(index + 1)];

					//minY, minX�� ����
					if ( start.m_element[y] > end.m_element[y] )
					{
						maxY = static_cast<int>( start.m_element[y] );
						minY = static_cast<int>( end.m_element[y] );
					}
					else
					{
						maxY = static_cast<int>( end.m_element[y] );
						minY = static_cast<int>( start.m_element[y] );
					}

					minX = min(start.m_element[x], end.m_element[x]);

					//���⸦ ����
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

					//edgeTable�� ����
					if ( m_edgeTable.find(minY) == m_edgeTable.end() )
					{
						m_edgeTable[minY] = std::vector<Edge>();
					}

					m_edgeTable[minY].emplace_back(maxY, minX, gradient);
				}
			}
		}
	}

	void CRasterizer::UpdateActiveEdgeTable(const int scanline)
	{
		//AET ���� ������� �ʴ� Edge����
		for ( auto iter = m_activeEdgeTable.begin(); iter != m_activeEdgeTable.end(); )
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
		else // AET �� Edge �߰�
		{
			std::vector<Edge>& edgelist = m_edgeTable.begin()->second;

			if ( m_edgeTable.begin()->first <= scanline )
			{
				for ( auto edge = edgelist.begin(); edge != edgelist.end(); ++edge )
				{
					m_activeEdgeTable.emplace_back(*edge);
				}
				m_edgeTable.erase(m_edgeTable.begin());
			}
		}
	}

	float CRasterizer::GetIntersectXpos(const int minY, const int scanlineY, const float minX, const float gradient) const
	{
		return minX + gradient * (scanlineY - minY);
	}
}