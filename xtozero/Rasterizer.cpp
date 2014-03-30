#include "stdafx.h"
#include "Rasterizer.h"
#include "Comman.h"

namespace xtozero
{
	void CRasterizer::CreateEdgeTable( CRsElementDesc& rsInput, int faceNumber )
	{
		if ( faceNumber >= rsInput.m_faces.size( ) )
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
		std::vector<int>& faces = rsInput.m_faces[faceNumber];

		for ( std::vector<int>::iterator& index = faces.begin( ); index != faces.end( ); ++index )
		{
			if ( (index + 1) == faces.end( ) )
			{
				//선분이 성립 안 됨.
				//Do Nothing
				break;
			}
			else
			{
				const Vector3& start = rsInput.m_vertices[*index];
				const Vector3& end = rsInput.m_vertices[*(index + 1)];

				//기울기를 구함
				dy = end.Y - start.Y;
				dx = end.X - start.X;

				if ( dy == 0 ) // 무조건 닫힌 도형이라면 x축에 평행한다면 빼도 될텐데...
				{
					gradient = 0.0f;
				}
				else
				{
					gradient = dx / dy;
				}

				//minY, minX를 구함
				if ( start.Y > end.Y )
				{
					maxY = static_cast<int>(start.Y);
					minY = static_cast<int>(end.Y);
				}
				else
				{
					maxY = static_cast<int>(end.Y);
					minY = static_cast<int>(start.Y);
				}

				minX = min( start.X, end.X );

				//edgeTable에 삽입

				m_edgeTable.emplace_back( minY, maxY, minX, gradient );
			}
		}
		//m_edgeTable을 minY로 정렬
		std::sort( m_edgeTable.begin(), m_edgeTable.end(),
			[]( const Edge& lhs, const Edge& rhs ) -> bool
			{
				return lhs.m_minY < rhs.m_minY;
			});
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

		while ( true )
		{
			if ( m_edgeTable.empty() )
			{
				break;
			}
			else
			{
				if ( m_edgeTable.begin( )->m_minY < scanline )
				{
					m_activeEdgeTable.emplace_back( *m_edgeTable.begin( ) );
					m_edgeTable.erase( m_edgeTable.begin( ) );
				}
				else
				{
					break;
				}
			}
		}

		//minX로 정렬
		if ( m_activeEdgeTable.size() > 1 )
		{
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

			startX = *iter;
			endX = *(iter + 1);
			if ( *iter > *(iter + 1) )
			{
				startX = *(iter + 1);
				endX = *iter;
			}

			if ( startX < m_viewport.m_left )
			{
				startX = m_viewport.m_left;
			}

			if ( endX > m_viewport.m_right )
			{
				endX = m_viewport.m_right;
			}

			for ( int i = startX; i <= endX; ++i )
			{
				m_outputRS.emplace_back( i, scanline, facecolor );
			}
		}
	}

	void CRasterizer::Process( CRsElementDesc& rsInput )
	{
		//여기에서 메시내부의 픽셀을 계산

		for ( std::vector<Vector3>::iterator& iter = rsInput.m_vertices.begin(); iter != rsInput.m_vertices.end(); ++iter )
		{
			(*iter).X = ((*iter).X * m_viewport.m_right * 0.5f) + m_viewport.m_right * 0.5f;
			(*iter).Y = -((*iter).Y * m_viewport.m_bottom * 0.5f) + m_viewport.m_bottom * 0.5f;
		}

		for ( int i = 0; i < rsInput.m_faces.size( ); ++i )
		{
			CreateEdgeTable( rsInput, i );

			int scanline = m_edgeTable.begin( )->m_minY;
			unsigned int facecolor = RAND_COLOR();

			while ( !(m_edgeTable.empty( ) && m_activeEdgeTable.empty( )) )
			{
				UpdateActiveEdgeTable( scanline );

				ProcessScanline( scanline, facecolor );

				++scanline;
			}
		}
	}

	void CRasterizer::SetViewPort( int left, int top, int right, int bottom )
	{
		if ( left > right )
		{
			Swap( left, right );
		}
		if ( top > bottom )
		{
			Swap( top, bottom );
		}
		
		if ( left < 0 )
		{
			m_viewport.m_left = 0;
		}
		if ( top < 0 )
		{
			m_viewport.m_top = 0;
		}

		m_viewport.m_left = left;
		m_viewport.m_top = top;
		m_viewport.m_right = right;
		m_viewport.m_bottom = bottom;

		m_outputRS.reserve( (right - left) * (bottom - top) );
	}
}