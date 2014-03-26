#include "CoolD_AreaFilling.h"
#include "CoolD_Inlines.h"
#include "CoolD_CustomMesh.h"

namespace CoolD
{
	AreaFilling::AreaFilling( void* buffer, const int width, const int height )
		: m_Buffer( (Buffer*)buffer ), m_Width(width), m_Height(height)
	{
	}

	AreaFilling::~AreaFilling()
	{			
		m_ListMesh.clear();
		m_ListLine.clear();
		m_ActiveTable.clear();
		m_EdgeTable.clear();
	}

	void AreaFilling::AddMesh( CustomMesh* const pMesh)
	{
		m_ListMesh.push_back( pMesh );
	}

	void AreaFilling::Render()
	{
		assert(!m_ListMesh.empty());	//비어 있으면 당연히 안됨

		for( auto& mesh : m_ListMesh )
		{			
			TimeForm start = chrono::system_clock::now();


			for( Duint faceNum = 1; faceNum <= mesh->GetFaceSize(); ++faceNum )
			{
				const BaseFace& face = CreatePointsToLines(mesh, faceNum);	//메쉬에서 어떤 면을 그릴 것 인지
				CreateEdgeTable( );	
				CreateChainTable( );
				DrawFace( MixDotColor(face.color) );
			}

			TimeForm end = chrono::system_clock::now();
			chrono::milliseconds mill = chrono::duration_cast<chrono::milliseconds>(end - start);		//test 시간 측정				
		}
	}

	//-----------------------------------------------------------------------
	//점 -> 선 만들기
	//-----------------------------------------------------------------------
	const BaseFace& AreaFilling::CreatePointsToLines(const CustomMesh* pMesh, Duint faceNum)
	{		
		const BaseFace& face = pMesh->GetFace( faceNum );

		for( Duint j = 0; j < face.vecIndex.size(); ++j )
		{
			LineKey lineKey;
			lineKey.beginIndex = face.vecIndex[ j ];
			BaseVertex beginVertex = pMesh->GetVertex(lineKey.beginIndex);
			
			if( j == face.vecIndex.size() - 1 )
			{
				lineKey.endIndex = face.vecIndex[ j % (face.vecIndex.size() - 1) ];
			}
			else
			{
				lineKey.endIndex = face.vecIndex[ j + 1 ];
			}
			BaseVertex endVertex = pMesh->GetVertex(lineKey.endIndex);

			m_ListLine.emplace_back( lineKey, beginVertex, endVertex );	//임시 객체 생성하면서 삽입
		}

		return face;
	}

	//-----------------------------------------------------------------------
	//엣지 테이블 만들기
	//-----------------------------------------------------------------------
	void AreaFilling::CreateEdgeTable( )
	{
		assert( !m_ListLine.empty() );	//비어 있으면 당연히 안됨

		for(const auto& line : m_ListLine )
		{
			EdgeNode node;
			Dfloat dx = line.endVertex.x - line.beginVertex.x;
			Dfloat dy = line.endVertex.y - line.beginVertex.y;

			if( dx == 0 || dy == 0 )
			{	//분모가 0이면 무조건 0 ZeroDivide방지
				node.reverseSlope = 0;
			}
			else
			{
				node.reverseSlope = dx / dy; // dx/dy가 기울기이고 그 역수값을 저장해야하기 때문에
			}

			node.x_min = (line.beginVertex.y < line.endVertex.y) ? line.beginVertex.x : line.endVertex.x;
			node.y_max = (line.beginVertex.y < line.endVertex.y) ? line.endVertex.y : line.beginVertex.y;

			m_EdgeTable.emplace_back( line.lineKey, node );
		}
	}

	//-----------------------------------------------------------------------
	//체인 테이블 생성 및 PreProcessing
	//-----------------------------------------------------------------------
	void AreaFilling::CreateChainTable( )
	{
		assert( !m_ListLine.empty() );	//비어 있으면 당연히 안됨

		int totalEdgeSize = m_ListLine.size();
		list<int> upLineSavelist;	//한 번이라도 윗줄로 올라간 정점들 인덱스 저장

		m_ListLine.sort([] (const Line& lhs, const Line& rhs) {	return lhs.lineKey.beginIndex < rhs.lineKey.beginIndex;	 });	//인덱스 순으로 오름차순 정렬

		for( int y = 0; y < m_Height; ++y )
		{
			list<Line> currentLine;			

			for( auto& lineIter = m_ListLine.begin(); lineIter != m_ListLine.end(); )
			{			
				if( y == (*lineIter).beginVertex.y || y == (*lineIter).endVertex.y ) //해당 y축에 정점이 걸치는지 검사			 	
				{
					currentLine.emplace_back( *lineIter );
					lineIter = m_ListLine.erase(lineIter);
				}
				else
				{
					++lineIter;
				}
			} // 현재 라인에 포함가능한 엣지들 모두 포함

			for( auto& chainIter = currentLine.begin(); chainIter != currentLine.end(); )
			{
				bool isOnceCount = true;
				for( auto& innerIter : currentLine )
				{
					if( (*chainIter).lineKey == innerIter.lineKey )	//같으면 리턴
					{
						continue;
					}

					if( (*chainIter).lineKey.beginIndex % totalEdgeSize > innerIter.lineKey.beginIndex % totalEdgeSize )	 //인덱스로 그려진 순서 비교
					{
						isOnceCount = false;
						continue;
					}

					if( CheckContinueLine( (*chainIter).lineKey, innerIter.lineKey) )	 //서로 연결되어있으면 일반적인 경우
					{
						isOnceCount = false;
						break;
					}
					isOnceCount = true;
				}

				if( isOnceCount )
				{
					if( find(upLineSavelist.begin(), upLineSavelist.end(), (*chainIter).lineKey.beginIndex) == upLineSavelist.end() )	//리스트에 포함되지 않았다 즉, 올라간적 없다.
					{						
						upLineSavelist.push_back( (*chainIter).lineKey.beginIndex );	//한번 올라간 내역 저장
						++(*chainIter).GetMinY();			//y축 값 1올리기								
						m_ListLine.emplace_back( *chainIter );//다시 검색하기 위해서 리스트에 넣어두자						
						chainIter = currentLine.erase( chainIter );
					}
					else
					{	
						++chainIter;
					}
				}
				else
				{					
					++chainIter;
				}
			}

			//currentLine.erase(remove_if(currentLine.begin(), currentLine.end(), [] (const Line& l){ 	return l.beginVertex.y == l.endVertex.y; }), currentLine.end());			
			STD_ERASE(currentLine, STD_REMOVE_IF(currentLine, [] (const Line& l){ return l.beginVertex.y == l.endVertex.y; }));

			if( !currentLine.empty() )
			{
				m_ActiveTable.emplace_back( y, currentLine );
			}
		}		
	}
	
	//-------------------------------------------------------------------
	//해당 위치 점 하나 찍기
	//-------------------------------------------------------------------
	void AreaFilling::DrawDot(const Duint x, const Duint y, const Dulong DotColor)
	{
		Duchar red = (DotColor >> 24);
		Duchar green = (DotColor >> 16) & 0x000000ff;
		Duchar blue = (DotColor >> 8) & 0x000000ff;
		Duchar currentcolor = 0;

		for( Duint k = 0; k < 3; ++k )
		{
			if( k == 0 ) currentcolor = red;
			else if( k == 1 ) currentcolor = green;
			else if( k == 2 ) currentcolor = blue;

			*(m_Buffer + (((m_Width * y) + x) * 3) + k) = currentcolor;
		}
	}

	//-------------------------------------------------------------------
	//현재 라인의 노드 사이 점 찍기
	//-------------------------------------------------------------------
	void AreaFilling::DrawLine( list<EdgeNode>& renderLine, const Dint currentHeight, const Dulong dotColor)
	{
		Dint beginX = -1;
		Dint endX = -1;
		Dint odd_even = 0;
		for( auto& dotNode : renderLine )
		{
			dotNode.x_min += dotNode.reverseSlope;

			if( odd_even % 2 == 0 ) //짝수
			{
				beginX = (int)ceilf(dotNode.x_min); //무조건 올림				
			}
			else //홀수			
			{
				endX = (int)ceilf(dotNode.x_min);

				for( Dint i = beginX; i <= endX; ++i )
				{					
					DrawDot(i, currentHeight, dotColor);
				}
			}
			++odd_even;
		}
	}	
	
	//-------------------------------------------------------------------
	// 메쉬의 해당 면 그리기
	//-------------------------------------------------------------------
	void AreaFilling::DrawFace(const Dulong dotColor)
	{
		assert( !m_ActiveTable.empty() );

		list<EdgeNode> continueRenderLine;
		auto& activeIter = m_ActiveTable.begin();

		m_ActiveTable.sort([] (const ActiveLine& lhs, const ActiveLine& rhs) -> bool { return lhs.height < rhs.height; } );
		for( Dint y = (*m_ActiveTable.begin()).height;; ++y )	//한줄한줄 그려나가야 하기 때문에 
		{
			if( y == (*activeIter).height )	//라인 정점과 높이가 겹치는 부분
			{
				for( auto& line : (*activeIter).currentLine ) //현재 높이에 걸쳐진 라인들을 순회
				{					
					auto& etIter = STD_FIND_IF(m_EdgeTable, [&line] (const LineEdge& le) { return le.lineKey == line.lineKey; });
					if( etIter != m_EdgeTable.end() )
					{
						continueRenderLine.emplace_back((*etIter).edgeNode);
					}
				}

				continueRenderLine.sort([] (const EdgeNode& lhs, const EdgeNode& rhs)
				{
					if( lhs.x_min == rhs.x_min )
					{	//기울기가 0인 경우가 있기 때문에 기울기 값만으로는 순서를 결정할수 없어서 x_min과 더한 결과값으로 비교한다.
						return lhs.x_min + lhs.reverseSlope < rhs.x_min + rhs.reverseSlope;
					}
					return lhs.x_min < rhs.x_min;
				});	//x_min값 크기 순으로 오름차순 정렬

				if( activeIter != (--m_ActiveTable.end()) )	//검사 대상을 다음번 위 정점으로 이동
				{
					++activeIter;
				}
			}

			DrawLine(continueRenderLine, y, dotColor);	//현재 라인 도트 찍기

			//continueRenderLine.erase( remove_if(continueRenderLine.begin(), continueRenderLine.end(),
			//	[y] (const EdgeNode& node)	{	return node.y_max == y; }), continueRenderLine.end() );	//y_max값이 현재 y라인과 같다면 리스트에서 삭제			
			STD_ERASE(continueRenderLine, STD_REMOVE_IF(continueRenderLine, [y] (const EdgeNode& node){return node.y_max == y; }));
			//--------------------------------
			if( continueRenderLine.size() == 0 )	//한 면이 그려지는 시점에는 항상 이 리스트에 값이 존재한다. 없을경우는 끝난경우
			{
				m_ActiveTable.clear();
				m_EdgeTable.clear();
				m_ListLine.clear();
				break;
			}
		}
	}

	//-------------------------------------------------------------------
	//각RGBA를 조합하여 컬러 만들어내기
	//-------------------------------------------------------------------
	Dulong AreaFilling::MixDotColor(const BaseColor& color)
	{
		return (color.r << 24) + (color.g << 16) + (color.b << 8) + (color.a);
	}

	//-------------------------------------------------------------------
	//라인 연결 여부 확인
	//-------------------------------------------------------------------
	
	bool AreaFilling::CheckContinueLine(const LineKey& lhs, const LineKey& rhs) const
	{	
		if( lhs.beginIndex	== rhs.endIndex ||			
			lhs.endIndex	== rhs.beginIndex )
		{
			return true;
		}

		return false;
	}	
}