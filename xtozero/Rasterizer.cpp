#include "stdafx.h"
#include "Rasterizer.h"
#include "Comman.h"

namespace xtozero
{
	void CRasterizer::CreateEdgeTable( const std::shared_ptr<CMesh> pMesh, int faceNumber )
	{
		if ( faceNumber >= pMesh->m_faces.size() )
		{
			return;
		}
		//To Do : EdgeTable�� �����.

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
				//������ ���� �� ��.
				//Do Nothing
				break;
			}
			else
			{
				const Vector3& start = pMesh->m_vertices[*index];
				const Vector3& end = pMesh->m_vertices[*(index + 1)];

				//���⸦ ����
				dy = end.Y - start.Y;
				dx = end.X - start.X;

				if ( dy == 0 ) // ������ ���� �����̶�� x�࿡ �����Ѵٸ� ���� ���ٵ�...
				{
					gradient = 0.0f;
				}
				else
				{
					gradient = dx / dy;
				}

				//minY, minX�� ����
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

				//edgeTable�� ����

				m_edgeTable.emplace_back( minY, maxY, minX, gradient );
			}
		}
		//m_edgeTable�� minY�� ����
		std::sort( m_edgeTable.begin(), m_edgeTable.end(),
			[]( const Edge& lhs, const Edge& rhs ) -> bool
			{
				return lhs.m_minY < rhs.m_minY;
			});
	}

	void CRasterizer::UpdateActiveEdgeTable( const int scanline )
	{
		//AET ���� ������� �ʴ� Edge����
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

		//minX�� ����
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

	void CRasterizer::ProcessScanline( int scanline, unsigned int facecolor )//���� �����ϸ� �÷� �Ѱ����� ���� ����...
	{
		std::vector<int> horizontalLine;

		//������ �׸� ������ ����

		int intersectX = 0;

		for ( std::vector<Edge>::iterator iter = m_activeEdgeTable.begin(); iter != m_activeEdgeTable.end(); ++iter )
		{
			if ( iter->m_maxY == iter->m_minY ) // ������ ������ ����
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

	void CRasterizer::Process( const std::shared_ptr<CMesh> pMesh )
	{
		//���⿡�� �޽ó����� �ȼ��� ���

		for ( int i = 0; i < pMesh->m_faces.size( ); ++i )
		{
			CreateEdgeTable( pMesh, i );

			int scanline = m_edgeTable.begin( )->m_minY;
			unsigned int facecolor = PIXEL_COLOR( pMesh->m_faces[i].m_color[r], pMesh->m_faces[i].m_color[g], pMesh->m_faces[i].m_color[b] );

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