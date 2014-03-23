#include "stdafx.h"
#include "Rasterizer.h"

namespace xtozero
{
	CRasterizer::CRasterizer( const std::shared_ptr<CMesh> pMesh )
	{
		//여기에서 메시내부의 픽셀을 계산

		for ( int i = 0; i < pMesh->m_faces.size(); ++i )
		{
			CreateEdgeTable( pMesh, i );

			int scanline = m_edgeTable.begin()->first;
			unsigned int facecolor = PIXEL_COLOR( pMesh->m_faces[i].m_color[r], pMesh->m_faces[i].m_color[g], pMesh->m_faces[i].m_color[b] );

			while ( !(m_edgeTable.empty() && m_activeEdgeTable.empty()) )
			{
				UpdateActiveEdgeTable( scanline );

				ProcessScanline( scanline, facecolor );

				++scanline;
			}
		}
	}

	CRasterizer::~CRasterizer( void )
	{

	}

	void CRasterizer::CreateEdgeTable( const std::shared_ptr<CMesh> pMesh, int faceNumber )
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

				//기울기를 구함
				dy = end.m_element[y] - start.m_element[y];
				dx = end.m_element[x] - start.m_element[x];

				if ( dy == 0 ) // 무조건 닫힌 도형이라면 x축에 평행한다면 빼도 될텐데...
				{
					gradient = 0.0f;
				}
				else
				{
					gradient = dx / dy;
				}

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

				minX = min( start.m_element[x], end.m_element[x] );

				//edgeTable에 삽입
				if ( m_edgeTable.find( minY ) == m_edgeTable.end() )
				{
					m_edgeTable[minY] = std::vector<Edge>();
				}

				m_edgeTable[minY].emplace_back( minY, maxY, minX, gradient );
			}
		}
	}

	void CRasterizer::UpdateActiveEdgeTable( const int scanline )
	{
		//AET 에서 사용하지 않는 Edge제거
		for ( std::vector<Edge>::iterator& iter = m_activeEdgeTable.begin(); iter != m_activeEdgeTable.end(); )
		{
			if ( iter->m_maxY < scanline )
			{
				iter = m_activeEdgeTable.erase( iter );
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

			if ( m_edgeTable.begin()->first < scanline )
			{
				for ( std::vector<Edge>::iterator& edge = edgelist.begin(); edge != edgelist.end(); ++edge )
				{
					m_activeEdgeTable.emplace_back( *edge );
				}
				m_edgeTable.erase( m_edgeTable.begin() );
			}

			//minX로 정렬
			std::sort( m_activeEdgeTable.begin(), m_activeEdgeTable.end(),
				[]( const Edge& lhs, const Edge& rhs )->bool
			{
				return lhs.m_minX < rhs.m_minX;
			} );
		}
	}

	float CRasterizer::GetIntersectXpos( int minY, int maxY, int scanlineY, float minX, float gradient ) const
	{
		if ( gradient > 0 )
		{
			return minX + (gradient * (scanlineY - minY));
		}
		else
		{
			return minX + (gradient * (scanlineY - maxY));
		}
	}

	void CRasterizer::ProcessScanline( int scanline, unsigned int facecolor )//정점 보간하면 컬러 넘겨주지 않을 예정...
	{
		std::vector<int> horizontalLine;

		//수평선을 그릴 구간을 지정

		int intersectX = 0;

		for ( std::vector<Edge>::iterator iter = m_activeEdgeTable.begin(); iter != m_activeEdgeTable.end(); ++iter )
		{
			if ( iter->m_maxY == iter->m_minY ) // 수평한 선분은 제외
			{

			}
			else
			{
				intersectX = GetIntersectXpos(
					iter->m_minY, iter->m_maxY, scanline, iter->m_minX, iter->m_gradient
					);

				horizontalLine.emplace_back( intersectX );
			}
		}

		int startX;
		int endX;
		for ( std::vector<int>::iterator iter = horizontalLine.begin(); iter != horizontalLine.end(); iter += 2 )
		{
			if ( (iter + 1) == horizontalLine.end() )
			{
				m_outputRS.emplace_back( *iter, scanline, facecolor );
				break;
			}

			if ( *iter > *(iter + 1) )
			{
				startX = *(iter + 1);
				endX = *iter;
			}
			else
			{
				startX = *iter;
				endX = *(iter + 1);
			}

			for ( int i = startX; i <= endX; ++i )
			{
				m_outputRS.emplace_back( i, scanline, facecolor );
			}
		}
	}
}