#include "stdafx.h"
#include "Rasterizer.h"
#include "Comman.h"

namespace xtozero
{
	cmd::CConvar g_BackfaceCulling( "BackfaceCulling", "1" );
	cmd::CConvar g_RandColor( "randColor", "0" );

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
		Vector3 startLocalVertex;
		Vector3 endLocalVertex;
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
				const Vector3& _startLocalVertex = rsInput.m_localVertices[faces.at( i )];;
				const Vector3& _endLocalVertex = rsInput.m_localVertices[faces.at( i + 1 )];;

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
				startLocalVertex = _startLocalVertex;
				endLocalVertex = _endLocalVertex;
				if ( minX > maxX )
				{
					minX = end.X;
					maxX = start.X;
					startZ = end.Z;
					endZ = start.Z;
					startLocalVertex = _endLocalVertex;
					endLocalVertex = _startLocalVertex;
				}

				//edgeTable에 삽입

				m_edgeTable.emplace_back( 
					minY, maxY, 
					minX, maxX, 
					startZ, endZ, gradient,
					startLocalVertex, endLocalVertex );
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
		for ( std::vector<Edge>::iterator iter = m_activeEdgeTable.begin(); iter != m_activeEdgeTable.end(); )
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

		for ( ; ; )
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

	void CRasterizer::ProcessScanline( int scanline, unsigned int facecolor )//정점 보간하면 컬러 넘겨주지 않을 예정...
	{
		if ( m_activeEdgeTable.empty( ) )
		{
			return;
		}
		m_horizontalLine.clear();
 
		//수평선을 그릴 구간을 지정

		float intersectX = 0;

		for ( std::vector<Edge>::iterator iter = m_activeEdgeTable.begin(); iter != m_activeEdgeTable.end(); ++iter )
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
				Vector3 localVertex = Lerp( edge.m_startLocalVertex, edge.m_endLocalVertex, lerpRatio );

				m_horizontalLine.emplace_back( static_cast<int>(intersectX), z, localVertex );
			}
		}

		int startX;
		int endX;
		float startZ;
		float endZ;
		Vector3 startLocalVertex;
		Vector3 endLocalVertex;
		for ( std::vector<std::tuple<int, float, Vector3>>::iterator iter = m_horizontalLine.begin(); iter != m_horizontalLine.end(); iter += 2 )
		{
			std::tuple<int, float, Vector3>& posXZ_LocalVertex = *iter;

			if ( (iter + 1) == m_horizontalLine.end() )
			{
				int x = std::get<0>( posXZ_LocalVertex );
				if ( x < m_viewport.m_left )
				{
					x = m_viewport.m_left;
				}
				else if ( x > m_viewport.m_right )
				{
					x = m_viewport.m_right;
				}

				m_outputRS.emplace_back( x, scanline, std::get<1>( posXZ_LocalVertex ), facecolor, std::get<2>( posXZ_LocalVertex ) );
				break;
			}

			std::tuple<int, float, Vector3>& posNextXZ_LocalVertex = *(iter + 1);

			startX = std::get<0>( posXZ_LocalVertex );
			endX = std::get<0>( posNextXZ_LocalVertex );
			startZ = std::get<1>( posXZ_LocalVertex );
			endZ = std::get<1>( posNextXZ_LocalVertex );
			startLocalVertex = std::get<2>( posXZ_LocalVertex );
			endLocalVertex = std::get<2>( posNextXZ_LocalVertex );
			if ( startX > endX )
			{
				startX = std::get<0>( posNextXZ_LocalVertex );
				endX = std::get<0>( posXZ_LocalVertex );
				startZ = std::get<1>( posNextXZ_LocalVertex );
				endZ = std::get<1>( posXZ_LocalVertex );
				startLocalVertex = std::get<2>( posNextXZ_LocalVertex );
				endLocalVertex = std::get<2>( posXZ_LocalVertex );
			}

			if ( startX < m_viewport.m_left )
			{
				startX = m_viewport.m_left;
			}

			if ( endX > m_viewport.m_right )
			{
				endX = m_viewport.m_right;
			}

			float z = startZ;
			Vector3 localVertex = startLocalVertex;
			
			float zGradient = (endZ - startZ) / (endX - startX);
			Vector3 localVertexGradient = (endLocalVertex - startLocalVertex) / (static_cast<float>(endX - startX));
			for ( int i = startX; i <= endX; ++i )
			{
				m_outputRS.emplace_back( i, scanline, z, facecolor, localVertex );
				z += zGradient;
				localVertex += localVertexGradient;
			}
		}
	}

	inline float CRasterizer::GetIntersectXpos( int minY, int maxY, int scanlineY, float minX, float gradient ) const
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

	const std::vector<CPsElementDesc>& CRasterizer::Process( CRsElementDesc& rsInput )
	{
		//여기에서 메시내부의 픽셀을 계산
		m_outputRS.clear();
		std::vector<bool> m_IsBack( rsInput.m_faces.size( ) );

		for ( unsigned int i = 0; i < rsInput.m_faces.size( ); ++i )
		{
			if ( IsBackFace( rsInput, i ) )
			{
				m_IsBack[i] = true;
			}
			else
			{
				m_IsBack[i] = false;
			}
		}

		if ( rsInput.m_coordinate == COORDINATE::OBJECT_COORDINATE )
		{
			for ( std::vector<Vector4>::iterator iter = rsInput.m_vertices.begin( ); iter != rsInput.m_vertices.end( ); ++iter )
			{
				Vector4& vertex = *iter;
				vertex /= vertex.W;

				vertex.X = (vertex.X * m_viewport.m_right * 0.5f) + m_viewport.m_right * 0.5f;
				vertex.Y = -(vertex.Y * m_viewport.m_bottom * 0.5f) + m_viewport.m_bottom * 0.5f;
			}
		}

		for ( unsigned int i = 0; i < rsInput.m_faces.size(); ++i )
		{
			if ( m_IsBack[i] && g_BackfaceCulling.GetBool( ) )
			{
				//Do Nothing
			}
			else
			{
				CreateEdgeTable( rsInput, i );

				int scanline = m_edgeTable.begin( )->m_minY;

				if ( scanline < m_viewport.m_top )
				{
					scanline = m_viewport.m_top;
				}

				//unsigned int facecolor = PIXEL_COLOR( 50, 204, 153 );
				unsigned int facecolor = RAND_COLOR( );

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
		}

		return m_outputRS;
	}

	const std::vector<CPsElementDesc>& CRasterizer::ProcessParallel( CRsElementDesc& rsInput, CXtzThreadPool* threadPool )
	{
		m_outputRS.clear( );

		if ( rsInput.m_coordinate == COORDINATE::OBJECT_COORDINATE )
		{
			for ( std::vector<Vector4>::iterator iter = rsInput.m_vertices.begin( ); iter != rsInput.m_vertices.end( ); ++iter )
			{
				Vector4& vertex = *iter;
				vertex /= vertex.W;

				vertex.X = (vertex.X * m_viewport.m_right * 0.5f) + m_viewport.m_right * 0.5f;
				vertex.Y = -(vertex.Y * m_viewport.m_bottom * 0.5f) + m_viewport.m_bottom * 0.5f;
			}
		}

		int threadFace = static_cast<int>(rsInput.m_faces.size( ) / threadPool->GetThreadNumber( ));
		int extra = rsInput.m_faces.size() % threadPool->GetThreadNumber();
		if ( extra != 0 )
		{
			threadFace++;
		}

		for ( unsigned int i = 0; i < threadPool->GetThreadNumber( ); ++i )
		{
			RsThreadArg* pRsArg = new RsThreadArg;

			pRsArg->pRs = this;
			pRsArg->startIdx = i * threadFace;
			pRsArg->endIdx = pRsArg->startIdx + threadFace;
			if ( pRsArg->endIdx > rsInput.m_faces.size() )
			{
				pRsArg->endIdx = rsInput.m_faces.size();
			}
			pRsArg->pRsElementDesc = &rsInput;

			threadPool->AddWork( RsThreadWork, (LPVOID)pRsArg );
		}

		threadPool->WaitThread();

		return m_outputRS;
	}

	const std::vector<CPsElementDesc>& CRasterizer::ProcessFaceRange( CRsElementDesc& rsInput, unsigned int startface, unsigned int endface )
	{
		//여기에서 메시내부의 픽셀을 계산
		m_outputRS.clear( );

		m_horizontalLine.reserve( 10 );

		for ( unsigned int i = startface; i < endface; ++i )
		{
			m_edgeTable.clear( );
			m_activeEdgeTable.clear( );

			if ( IsBackFace( rsInput, i ) && g_BackfaceCulling.GetBool( ) )
			{
				//Do Nothing
			}
			else
			{
				CreateEdgeTable( rsInput, i );

				int scanline = m_edgeTable.begin( )->m_minY;

				if ( scanline < m_viewport.m_top )
				{
					scanline = m_viewport.m_top;
				}

				unsigned int facecolor;
				if ( g_RandColor.GetBool( ) )
				{
					facecolor = RAND_COLOR( );
				}
				else
				{
					facecolor = PIXEL_COLOR( 255, 204, 153 );
				}

				while ( !(m_edgeTable.empty( ) && m_activeEdgeTable.empty( )) )
				{
					if ( scanline > m_viewport.m_bottom )
					{
						break;
					}

					UpdateActiveEdgeTable( scanline );

					ProcessScanline( scanline, facecolor );

					++scanline;
				}
			}
		}

		return m_outputRS;
	}

	void CRasterizer::CreateEdgeTableParallel( const CRsElementDesc& rsInput, unsigned int faceNumber, std::vector<Edge>& edgeTable )
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
		Vector3 startLocalVertex;
		Vector3 endLocalVertex;
		const std::vector<int>& faces = rsInput.m_faces[faceNumber];

		int nfaces = faces.size( );
		edgeTable.reserve( nfaces );
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
				const Vector3& _startLocalVertex = rsInput.m_localVertices[faces.at( i )];
				const Vector3& _endLocalVertex = rsInput.m_localVertices[faces.at( i + 1 )];

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
				startLocalVertex = _startLocalVertex;
				endLocalVertex = _endLocalVertex;
				if ( minX > maxX )
				{
					minX = end.X;
					maxX = start.X;
					startZ = end.Z;
					endZ = start.Z;
					startLocalVertex = _endLocalVertex;
					endLocalVertex = _startLocalVertex;
				}

				//edgeTable에 삽입

				edgeTable.emplace_back(
					minY, maxY,
					minX, maxX,
					startZ, endZ, gradient,
					startLocalVertex, endLocalVertex );
			}
		}
		//edgeTable을 minY로 정렬
		std::sort( edgeTable.begin( ), edgeTable.end( ), CompareEdgeY );
	}

	void CRasterizer::UpdateActiveEdgeTableParallel( const int scanline, std::vector<Edge>& edgeTable, std::vector<Edge>& activeEdgeTable )
	{
		//AET 에서 사용하지 않는 Edge제거
		for ( std::vector<Edge>::iterator iter = activeEdgeTable.begin( ); iter != activeEdgeTable.end( ); )
		{
			Edge& edge = *iter;
			if ( edge.m_maxY < scanline )
			{
				iter = activeEdgeTable.erase( iter );
			}
			else
			{
				++iter;
			}
		}

		for ( ; ; )
		{
			if ( edgeTable.empty( ) )
			{
				return;
			}
			else
			{
				if ( edgeTable.begin( )->m_minY < scanline )
				{
					activeEdgeTable.emplace_back( *edgeTable.begin( ) );
					edgeTable.erase( edgeTable.begin( ) );
				}
				else
				{
					return;
				}
			}
		}

		//minX로 정렬
		if ( activeEdgeTable.size( ) > 1 )
		{
			std::sort( activeEdgeTable.begin( ), activeEdgeTable.end( ), CompareEdgeX );
		}
	}

	void CRasterizer::ProcessScanlineParallel( int scanline, unsigned int facecolor, std::vector<Edge>& activeEdgeTable,
		std::vector<CPsElementDesc>& outputRS, std::vector < std::tuple < int, float, Vector3 >> &horizontalLine )//정점 보간하면 컬러 넘겨주지 않을 예정...
	{
		if ( activeEdgeTable.empty() )
		{
			return;
		}
		horizontalLine.clear();
		//수평선을 그릴 구간을 지정

		float intersectX = 0;

		for ( unsigned int i = 0; i < activeEdgeTable.size(); ++i )
		{
			Edge& edge = activeEdgeTable[i];
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
				Vector3 localVertex = Lerp( edge.m_startLocalVertex, edge.m_endLocalVertex, lerpRatio );

				horizontalLine.emplace_back( static_cast<int>(intersectX), z, localVertex );
			}
		}

		int startX;
		int endX;
		float startZ;
		float endZ;
		Vector3 startLocalVertex;
		Vector3 endLocalVertex;
		for ( unsigned int i = 0; i < horizontalLine.size(); i += 2 )
		{
			std::tuple<int, float, Vector3>& posXZ_LocalVertex = horizontalLine[i];

			if ( (i + 1) == horizontalLine.size( ) )
			{
				int x = std::get<0>( posXZ_LocalVertex );
				if ( x < m_viewport.m_left )
				{
					x = m_viewport.m_left;
				}
				else if ( x > m_viewport.m_right )
				{
					x = m_viewport.m_right;
				}

				outputRS.emplace_back( x, scanline, std::get<1>( posXZ_LocalVertex ), facecolor, std::get<2>( posXZ_LocalVertex ) );

				break;
			}

			std::tuple<int, float, Vector3>& posNextXZ_LocalVertex = horizontalLine[i + 1];

			startX = std::get<0>( posXZ_LocalVertex );
			endX = std::get<0>( posNextXZ_LocalVertex );
			startZ = std::get<1>( posXZ_LocalVertex );
			endZ = std::get<1>( posNextXZ_LocalVertex );
			startLocalVertex = std::get<2>( posXZ_LocalVertex );
			endLocalVertex = std::get<2>( posNextXZ_LocalVertex );
			if ( startX > endX )
			{
				startX = std::get<0>( posNextXZ_LocalVertex );
				endX = std::get<0>( posXZ_LocalVertex );
				startZ = std::get<1>( posNextXZ_LocalVertex );
				endZ = std::get<1>( posXZ_LocalVertex );
				startLocalVertex = std::get<2>( posNextXZ_LocalVertex );
				endLocalVertex = std::get<2>( posXZ_LocalVertex );
			}

			if ( startX < m_viewport.m_left )
			{
				startX = m_viewport.m_left;
			}

			if ( endX > m_viewport.m_right )
			{
				endX = m_viewport.m_right;
			}

			float z = startZ;
			Vector3 localVertex = startLocalVertex;
			float zGradient = (endZ - startZ) / (endX - startX);
			Vector3 localVertexGradient = (endLocalVertex - startLocalVertex) / (static_cast<float>(endX - startX));
			for ( int i = startX; i <= endX; ++i )
			{
				outputRS.emplace_back( i, scanline, z, facecolor, localVertex );
				z += zGradient;
				localVertex += localVertexGradient;
			}
		}
	}

	void CRasterizer::SetViewPort( int left, int top, int right, int bottom )
	{
		assert( left < right );
		assert( top < bottom );
		assert( left >= 0 );
		assert( top >= 0 );

		m_viewport.m_left = left;
		m_viewport.m_top = top;
		m_viewport.m_right = right - 1;
		m_viewport.m_bottom = bottom - 1;

		m_outputRS.reserve( (right - left) * (bottom - top) );
	}

	const std::vector<CPsElementDesc>& CBarycentricRasterizer::Process( CRsElementDesc& rsInput )
	{
		m_outputRS.clear();

		if ( rsInput.m_coordinate == COORDINATE::OBJECT_COORDINATE )
		{
			for ( std::vector<Vector4>::iterator iter = rsInput.m_vertices.begin( ); iter != rsInput.m_vertices.end( ); ++iter )
			{
				Vector4& vertex = *iter;
				vertex /= vertex.W;

				vertex.X = (vertex.X * m_viewport.m_right * 0.5f) + m_viewport.m_right * 0.5f;
				vertex.Y = -(vertex.Y * m_viewport.m_bottom * 0.5f) + m_viewport.m_bottom * 0.5f;
			}
		}

		for ( unsigned int i = 0; i < rsInput.m_faces.size(); ++i )
		{
			if ( IsBackFace( rsInput, i ) && g_BackfaceCulling.GetBool( ) )
			{
				//Do Nothing
			}
			else
			{
				unsigned int facecolor;
				if ( g_RandColor.GetBool( ) )
				{
					facecolor = RAND_COLOR( );
				}
				else
				{
					facecolor = PIXEL_COLOR( 255, 204, 153 );
				}

				std::vector<int>& face = rsInput.m_faces[i];

				const Vector4& v0 = rsInput.m_vertices[face[0]];
				const Vector4& v1 = rsInput.m_vertices[face[1]];
				const Vector4& v2 = rsInput.m_vertices[face[2]];

				const Vector3& localVertex0 = rsInput.m_localVertices[face[0]];
				const Vector3& localVertex1 = rsInput.m_localVertices[face[1]];
				const Vector3& localVertex2 = rsInput.m_localVertices[face[2]];

				//Bounding Area를 계산
				int maxX = static_cast<int>(max( min( max( max( v0.X, v1.X ), v2.X ), m_viewport.m_right ), 0 ));
				int maxY = static_cast<int>(max( min( max( max( v0.Y, v1.Y ), v2.Y ), m_viewport.m_bottom ), 0 ));
				int minX = static_cast<int>(max( min( min( v0.X, v1.X ), v2.X ), 0 ));
				int minY = static_cast<int>(max( min( min( v0.Y, v1.Y ), v2.Y ), 0 ));

				Vector4 p;

				bool IsInOnce;

				// 외적 공식에 의해서 한 점이 한 축으로 증가했을때 다음과 같이 증가한다.
				float s0IncX = v0.Y - v1.Y;
				float s1IncX = v1.Y - v2.Y;
				float s2IncX = v2.Y - v0.Y;
				float s0IncY = v1.X - v0.X;
				float s1IncY = v2.X - v1.X;
				float s2IncY = v0.X - v2.X;

				p.X = static_cast<float>(minX);
				p.Y = static_cast<float>(minY);

				float s0 = CalcParallelogramArea( p, v0, v1 );
				float s1 = CalcParallelogramArea( p, v1, v2 );
				float s2 = CalcParallelogramArea( p, v2, v0 );

				for ( int i = minY; i <= maxY; ++i )
				{
					IsInOnce = false;

					float tempS0 = s0;
					float tempS1 = s1;
					float tempS2 = s2;

					for ( int j = minX; j <= maxX; ++j )
					{
						if ( tempS0 >= 0 && tempS1 >= 0 && tempS2 >= 0 )
						{
							IsInOnce = true;

							float denominator = 1.0f / (tempS0 + tempS1 + tempS2);
							float tempU = tempS1 * denominator;
							float tempV = tempS2 * denominator;
							float tempW = tempS0 * denominator;

							float z = v0.Z * tempU + v1.Z * tempV + v2.Z * tempW;
							Vector3 localVertex = localVertex0 * tempU + localVertex1 * tempV + localVertex2 * tempW;

							m_outputRS.emplace_back( j, i, z, facecolor, localVertex );
						}
						else if ( IsInOnce == true )
						{
							// 삼각형에서 한번 삼각형 내부에 들어간 다음 외부로 나왔을 때
							// 다시 내부로 들어가는 일은 없다.
							break;
						}

						tempS0 += s0IncX;
						tempS1 += s1IncX;
						tempS2 += s2IncX;
					}

					s0 += s0IncY;
					s1 += s1IncY;
					s2 += s2IncY;
				}
			}
		}

		return m_outputRS;
	}

	const std::vector<CPsElementDesc>& CBarycentricRasterizer::ProcessFaceRange( CRsElementDesc& rsInput, unsigned int startface, unsigned int endface )
	{
		m_outputRS.clear( );

		for ( unsigned int i = startface; i < endface; ++i )
		{
			if ( IsBackFace( rsInput, i ) && g_BackfaceCulling.GetBool( ) )
			{
				//Do Nothing
			}
			else
			{
				unsigned int facecolor;
				if ( g_RandColor.GetBool( ) )
				{
					facecolor = RAND_COLOR( );
				}
				else
				{
					facecolor = PIXEL_COLOR( 255, 204, 153 );
				}

				std::vector<int>& face = rsInput.m_faces[i];

				const Vector4& v0 = rsInput.m_vertices[face[0]];
				const Vector4& v1 = rsInput.m_vertices[face[1]];
				const Vector4& v2 = rsInput.m_vertices[face[2]];

				const Vector3& localVertex0 = rsInput.m_localVertices[face[0]];
				const Vector3& localVertex1 = rsInput.m_localVertices[face[1]];
				const Vector3& localVertex2 = rsInput.m_localVertices[face[2]];

				//Bounding Area를 계산
				int maxX = static_cast<int>(max( min( max( max( v0.X, v1.X ), v2.X ), m_viewport.m_right ), 0 ));
				int maxY = static_cast<int>(max( min( max( max( v0.Y, v1.Y ), v2.Y ), m_viewport.m_bottom ), 0 ));
				int minX = static_cast<int>(max( min( min( v0.X, v1.X ), v2.X ), 0 ));
				int minY = static_cast<int>(max( min( min( v0.Y, v1.Y ), v2.Y ), 0 ));

				Vector4 p;

				bool IsInOnce;

				// 외적 공식에 의해서 한 점이 한 축으로 증가했을때 다음과 같이 증가한다.
				float s0IncX = v0.Y - v1.Y;
				float s1IncX = v1.Y - v2.Y;
				float s2IncX = v2.Y - v0.Y;
				float s0IncY = v1.X - v0.X;
				float s1IncY = v2.X - v1.X;
				float s2IncY = v0.X - v2.X;

				p.X = static_cast<float>(minX);
				p.Y = static_cast<float>(minY);

				float s0 = CalcParallelogramArea( p, v0, v1 );
				float s1 = CalcParallelogramArea( p, v1, v2 );
				float s2 = CalcParallelogramArea( p, v2, v0 );

				for ( int i = minY; i <= maxY; ++i )
				{
					IsInOnce = false;

					float tempS0 = s0;
					float tempS1 = s1;
					float tempS2 = s2;

					for ( int j = minX; j <= maxX; ++j )
					{
						if ( tempS0 >= 0 && tempS1 >= 0 && tempS2 >= 0 )
						{
							IsInOnce = true;

							float denominator = 1.0f / (tempS0 + tempS1 + tempS2);
							float tempU = tempS1 * denominator;
							float tempV = tempS2 * denominator;
							float tempW = tempS0 * denominator;

							float z = v0.Z * tempU + v1.Z * tempV + v2.Z * tempW;
							Vector3 localVertex = localVertex0 * tempU + localVertex1 * tempV + localVertex2 * tempW;

							m_outputRS.emplace_back( j, i, z, facecolor, localVertex );
						}
						else if ( IsInOnce == true )
						{
							// 삼각형에서 한번 삼각형 내부에 들어간 다음 외부로 나왔을 때
							// 다시 내부로 들어가는 일은 없다.
							break;
						}

						tempS0 += s0IncX;
						tempS1 += s1IncX;
						tempS2 += s2IncX;
					}

					s0 += s0IncY;
					s1 += s1IncY;
					s2 += s2IncY;
				}
			}
		}

		return m_outputRS;
	}

	void CBarycentricRasterizer::SetViewPort( int left, int top, int right, int bottom )
	{
		assert( left < right );
		assert( top < bottom );
		assert( left >= 0 );
		assert( top >= 0 );

		m_viewport.m_left = left;
		m_viewport.m_top = top;
		m_viewport.m_right = right - 1;
		m_viewport.m_bottom = bottom - 1;

		m_outputRS.reserve( (right - left) * (bottom - top) );
	}

	float CBarycentricRasterizer::CalcParallelogramArea( const Vector4& p, const Vector4& v0, const Vector4 v1 )
	{
		return (v1.X - v0.X) * (p.Y - v0.Y) - (v1.Y - v0.Y) * (p.X - v0.X);
	}

	bool IsBackFace( const CRsElementDesc& rsInput, const int facenumber )
	{
		const Vector4& v0 = rsInput.m_vertices[rsInput.m_faces[facenumber][0]];
		const Vector4& v1 = rsInput.m_vertices[rsInput.m_faces[facenumber][1]];
		const Vector4& v2 = rsInput.m_vertices[rsInput.m_faces[facenumber][2]];

		Vector4 s0 = v1 - v0;
		Vector4 s1 = v2 - v0;

		s1 = s1.CrossProduct( s0 ); // 법선 벡터;

		if ( s1.DotProduct( Vector4( 0.0f, 0.0f, -1.0f) ) < 0 )
		{
			return true;
		}

		return false;
	}

	void RsThreadWork( LPVOID arg )
	{
		RsThreadArg* pRsArg = (RsThreadArg*)arg;

		CRasterizer* rasterizer = pRsArg->pRs;

		std::vector<Edge> edgeTable;
		std::vector<Edge> activeEdgeTable;
		std::vector<CPsElementDesc> outputRS;
		std::vector<std::tuple<int, float, Vector3>> horizontalLine;

		Rect& viewport = rasterizer->GetViewport();
		int nResv = static_cast<int>( rasterizer->m_outputRS.capacity() * 0.25 );

		outputRS.reserve( nResv );

		for ( unsigned int i = pRsArg->startIdx; i < pRsArg->endIdx; ++i )
		{
			if ( IsBackFace( *pRsArg->pRsElementDesc, i ) && g_BackfaceCulling.GetBool() )
			{
				continue;
			}
			else
			{
				edgeTable.clear( );
				activeEdgeTable.clear( );
				rasterizer->CreateEdgeTableParallel( *pRsArg->pRsElementDesc, i, edgeTable );

				activeEdgeTable.reserve( edgeTable.size( ) );
				horizontalLine.reserve( edgeTable.size( ) );

				int scanline = edgeTable.begin( )->m_minY;

				if ( scanline < viewport.m_top )
				{
					scanline = viewport.m_top;
				}

				unsigned int facecolor;
				if ( g_RandColor.GetBool( ) )
				{
					facecolor = RAND_COLOR( );
				}
				else
				{
					facecolor = PIXEL_COLOR( 255, 204, 153 );
				}

				while ( !(edgeTable.empty( ) && activeEdgeTable.empty( )) )
				{
					if ( scanline > viewport.m_bottom )
					{
						break;
					}

					rasterizer->UpdateActiveEdgeTableParallel( scanline, edgeTable, activeEdgeTable );

					rasterizer->ProcessScanlineParallel( scanline,
						facecolor,
						activeEdgeTable,
						outputRS,
						horizontalLine );

					++scanline;
				}
			}
		}

		{
			Lock<SpinLock> lock( rasterizer->GetLockObject() );
			rasterizer->m_outputRS.insert( rasterizer->m_outputRS.end(),
				outputRS.begin(), outputRS.end() );
		}

		delete pRsArg;
	}
}