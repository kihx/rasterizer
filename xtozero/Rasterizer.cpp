#include "stdafx.h"
#include "Rasterizer.h"
#include "Comman.h"

namespace xtozero
{
	void CRasterizer::CreateEdgeTable( CRsElementDesc& rsInput, unsigned int faceNumber )
	{
		if ( faceNumber >= rsInput.m_faces.size( ) )
		{
			return;
		}
		//To Do : EdgeTable�� �����.

		int minY = 0;
		int maxY = 0;
		float minX = 0.0f;
		float maxX = 0.0f;
		float startZ = 0.0f;
		float endZ = 0.0f;
		float gradient = 0.0f;
		float dx = 0.0f;
		float dy = 0.0f;
		std::vector<int>& faces = rsInput.m_faces[faceNumber];

		for ( std::vector<int>::iterator& index = faces.begin( ); index != faces.end( ); ++index )
		{
			if ( (index + 1) == faces.end( ) )
			{
				//������ ���� �� ��.
				//Do Nothing
				break;
			}
			else
			{
				const Vector4& start = rsInput.m_vertices[*index];
				const Vector4& end = rsInput.m_vertices[*(index + 1)];

				//���⸦ ����
				dy = end.Y - start.Y;
				dx = end.X - start.X;

				if ( -1.0 < dy && dy < 1.0 ) //dy�� 0.2�� ���� ���·� ���´ٸ� ���Ⱑ ū������ �þ��.
				{
					gradient = 0.0f;
				}
				else
				{
					gradient = dx / dy;
				}

				//minY, minX�� ����
				maxY = static_cast<int>(end.Y);
				minY = static_cast<int>(start.Y);

				if ( start.Y > end.Y )
				{
					maxY = static_cast<int>(start.Y);
					minY = static_cast<int>(end.Y);
				}

				minX = start.X;
				maxX = end.X;
				startZ = start.Z;
				endZ = end.Z;
				if ( minX > maxX )
				{
					minX = end.X;
					maxX = start.X;
					startZ = end.Z;
					endZ = start.Z;
				}

				//edgeTable�� ����

				m_edgeTable.emplace_back( 
					minY, maxY, 
					minX, maxX, 
					startZ, endZ, gradient );
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
		std::vector<std::pair<int, float>> horizontalLine;
 
		//������ �׸� ������ ����

		float intersectX = 0;

		for ( std::vector<Edge>::iterator& iter = m_activeEdgeTable.begin(); iter != m_activeEdgeTable.end(); ++iter )
		{
			if ( iter->m_maxY == iter->m_minY ) // ������ ������ ����
			{

			}
			else
			{
				intersectX = GetIntersectXpos(
					iter->m_minY, iter->m_maxY, scanline, iter->m_minX, iter->m_gradient
					);

				// y�࿡ ���� ����� x�࿡ ���� ���е��� ��������.
				// ���� ���� �������� maxX ���� ũ�� maxX�� �����Ѵ�.
				if ( intersectX > iter->m_maxX ) 
				{
					intersectX = iter->m_maxX;
				}

				float lerpRatio = 1.0f;
				if ( (iter->m_maxX - iter->m_minX) == 0 )
				{
					//Do Nothing
				}
				else
				{
					lerpRatio = (intersectX - iter->m_minX) / (iter->m_maxX - iter->m_minX);
				}

				float z = Lerp( iter->m_startZ, iter->m_endZ, lerpRatio );

				horizontalLine.emplace_back( std::make_pair( static_cast<int>(ceilf( intersectX )), z ) );
			}
		}

		int startX;
		int endX;
		float startZ;
		float endZ;
		for ( std::vector<std::pair<int, float>>::iterator& iter = horizontalLine.begin(); iter != horizontalLine.end(); iter += 2 )
		{
			if ( (iter + 1) == horizontalLine.end() )
			{
				m_outputRS.emplace_back( iter->first, scanline, iter->second, facecolor );
				break;
			}

			startX = iter->first;
			endX = (iter + 1)->first;
			startZ = iter->second;
			endZ = (iter + 1)->second;
			if ( startX > endX )
			{
				startX = (iter + 1)->first;
				endX = iter->first;
				startZ = (iter + 1)->second;
				endZ = iter->second;
			}

			if ( startX < m_viewport.m_left )
			{
				startX = m_viewport.m_left;
			}

			if ( endX >= m_viewport.m_right )
			{
				endX = m_viewport.m_right - 1;
			}

			for ( int i = startX; i <= endX; ++i )
			{
				float lerpRatio = 1.0f;
				if ( endX == startX )
				{
					//Do Nothing
				}
				else
				{
					lerpRatio = static_cast<float>( i - startX ) / (endX - startX);
				}
				float z = Lerp( startZ, endZ, lerpRatio );
				m_outputRS.emplace_back( i, scanline, z, facecolor );
			}
		}
	}

	std::vector<CPsElementDesc> CRasterizer::Process( CRsElementDesc& rsInput )
	{
		//���⿡�� �޽ó����� �ȼ��� ���
		m_outputRS.clear();

		if ( rsInput.m_coodinate == COORDINATE::OBJECT_COORDINATE )
		{
			for ( std::vector<Vector4>::iterator& iter = rsInput.m_vertices.begin( ); iter != rsInput.m_vertices.end( ); ++iter )
			{
				(*iter).X /= (*iter).W;
				(*iter).Y /= (*iter).W;
				(*iter).Z /= (*iter).W;
				(*iter).W /= (*iter).W;

				(*iter).X = ((*iter).X * m_viewport.m_right * 0.5f) + m_viewport.m_right * 0.5f;
				(*iter).Y = -((*iter).Y * m_viewport.m_bottom * 0.5f) + m_viewport.m_bottom * 0.5f;
			}
		}

		for ( unsigned int i = 0; i < rsInput.m_faces.size(); ++i )
		{
			CreateEdgeTable( rsInput, i );

			int scanline = m_edgeTable.begin( )->m_minY;

			if ( scanline < m_viewport.m_top )
			{
				scanline = m_viewport.m_top;
			}

			//unsigned int facecolor = PIXEL_COLOR( 0, 255, 255 );
			unsigned int facecolor = RAND_COLOR();

			while ( !(m_edgeTable.empty( ) && m_activeEdgeTable.empty( )) )
			{
				UpdateActiveEdgeTable( scanline );

				ProcessScanline( scanline, facecolor );

				++scanline;

				if ( scanline > m_viewport.m_bottom )
				{
					break;
				}
			}
		}

		return m_outputRS;
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