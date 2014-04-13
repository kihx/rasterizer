#include "stdafx.h"
#include "Rasterizer.h"
#include "Comman.h"

namespace xtozero
{
	void CRasterizer::CreateEdgeTable( const CRsElementDesc& rsInput, unsigned int faceNumber )
	{
		if ( faceNumber >= rsInput.m_faces.size( ) )
		{
			return;
		}
		//To Do : EdgeTable을 만든다.

		int minY = 0;
		int maxY = 0;
		float minX = 0.0f;
		float maxX = 0.0f;
		float startZ = 0.0f;
		float endZ = 0.0f;
		float gradient = 0.0f;
		float dx = 0.0f;
		float dy = 0.0f;
		const std::vector<int>& faces = rsInput.m_faces[faceNumber];

		int nfaces = faces.size();
		for ( int i = 0; i < nfaces; ++i )
		{
			if ( (i + 1) == nfaces )
			{
				//선분이 성립 안 됨.
				//Do Nothing
				break;
			}
			else
			{
				const Vector4& start = rsInput.m_vertices[faces.at( i )];
				const Vector4& end = rsInput.m_vertices[faces.at( i + 1 )];

				//기울기를 구함
				dy = end.Y - start.Y;
				dx = end.X - start.X;

				if ( -1.0 < dy && dy < 1.0 ) //dy가 0.2와 같은 형태로 나온다면 기울기가 큰폭으로 늘어난다.
				{
					gradient = 0.0f;
				}
				else
				{
					gradient = dx / dy;
				}

				//minY, minX를 구함
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

				//edgeTable에 삽입

				m_edgeTable.emplace_back( 
					minY, maxY, 
					minX, maxX, 
					startZ, endZ, gradient );
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
			Edge& edge = *iter;
			if ( edge.m_maxY < scanline )
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
				return;
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
					return;
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
		std::vector<std::pair<int, float>> horizontalLine;
		horizontalLine.reserve( m_activeEdgeTable.size() );
 
		//수평선을 그릴 구간을 지정

		float intersectX = 0;

		for ( std::vector<Edge>::iterator& iter = m_activeEdgeTable.begin(); iter != m_activeEdgeTable.end(); ++iter )
		{
			Edge& edge = *iter;
			if ( edge.m_maxY == edge.m_minY ) // 수평한 선분은 제외
			{

			}
			else
			{
				intersectX = GetIntersectXpos(
					edge.m_minY, edge.m_maxY, scanline, edge.m_minX, edge.m_gradient
					);

				// y축에 대한 기울기는 x축에 대한 정밀도가 떨어진다.
				// 따라서 구한 교차점이 maxX 보다 크면 maxX로 강제한다.
				if ( intersectX > edge.m_maxX )
				{
					intersectX = edge.m_maxX;
				}

				float lerpRatio = 1.0f;
				if ( (edge.m_maxX - edge.m_minX) == 0 )
				{
					//Do Nothing
				}
				else
				{
					lerpRatio = (intersectX - edge.m_minX) / (edge.m_maxX - edge.m_minX);
				}

				float z = Lerp( edge.m_startZ, edge.m_endZ, lerpRatio );

				horizontalLine.emplace_back( ceilf( intersectX ), z );
			}
		}

		int startX;
		int endX;
		float startZ;
		float endZ;
		for ( std::vector<std::pair<int, float>>::iterator& iter = horizontalLine.begin(); iter != horizontalLine.end(); iter += 2 )
		{
			std::pair<int, float>& posXZ = *iter;

			if ( (iter + 1) == horizontalLine.end() )
			{
				m_outputRS.emplace_back( posXZ.first, scanline, posXZ.second, facecolor );
				break;
			}

			std::pair<int, float>& posNextXZ = *(iter + 1);

			startX = posXZ.first;
			endX = posNextXZ.first;
			startZ = posXZ.second;
			endZ = posNextXZ.second;
			if ( startX > endX )
			{
				startX = posNextXZ.first;
				endX = posXZ.first;
				startZ = posNextXZ.second;
				endZ = posXZ.second;
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

				if ( z > 1.0f || z < 0.0f )
				{

				}
				else
				{
					m_outputRS.emplace_back( i, scanline, z, facecolor );
				}	
			}
		}
	}

	const std::vector<CPsElementDesc>& CRasterizer::Process( CRsElementDesc& rsInput )
	{
		//여기에서 메시내부의 픽셀을 계산
		m_outputRS.clear();

		if ( rsInput.m_coodinate == COORDINATE::OBJECT_COORDINATE )
		{
			for ( std::vector<Vector4>::iterator& iter = rsInput.m_vertices.begin( ); iter != rsInput.m_vertices.end( ); ++iter )
			{
				Vector4& vertex = *iter;
				vertex /= vertex.W;

				vertex.X = (vertex.X * m_viewport.m_right * 0.5f) + m_viewport.m_right * 0.5f;
				vertex.Y = -(vertex.Y * m_viewport.m_bottom * 0.5f) + m_viewport.m_bottom * 0.5f;
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

			//unsigned int facecolor = PIXEL_COLOR( 50, 204, 153 );
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

	const std::vector<CPsElementDesc>& CRasterizer::ProcessParallel( CRsElementDesc& rsInput )
	{
		m_outputRS.clear( );
	}

	void CRasterizer::SetViewPort( int left, int top, int right, int bottom )
	{
		assert( left < right );
		assert( top < bottom );
		assert( left >= 0 );
		assert( top >= 0 );

		m_viewport.m_left = left;
		m_viewport.m_top = top;
		m_viewport.m_right = right;
		m_viewport.m_bottom = bottom;

		m_outputRS.reserve( (right - left) * (bottom - top) );
	}
}