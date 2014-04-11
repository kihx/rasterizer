#include "CoolD_AreaFilling.h"
#include "..\Data\CoolD_Inlines.h"
#include "CoolD_CustomMesh.h"
#include "CoolD_Transform.h"

namespace CoolD
{
	AreaFilling::AreaFilling()
		: m_Buffer(nullptr), m_DepthBuffer(nullptr)
	{
	}

	AreaFilling::~AreaFilling()
	{					
		Clear();
		Safe_Delete_Array(m_DepthBuffer);
	}

	Dvoid AreaFilling::Render( tuple_meshInfo& meshInfo)
	{							
		//TimeForm start = chrono::system_clock::now();
		
		for( Duint faceNum = 0; faceNum < get<1>(meshInfo).size(); ++faceNum )
		{
			tuple_OptimizeY opY = CreatePointsToLines(meshInfo, faceNum);
			if (CheckYStandInLine())
			{
				Clear();
				continue;
			}

			CreateEdgeTable( );		
			CreateChainTable(opY);			
			
			RandomGenerator<int> rand(0, 255);
			BaseColor color;
			color = { rand.GetRand(), rand.GetRand(), rand.GetRand(), rand.GetRand() };

			DrawFace( MixDotColor(color) );
		}
		
		//TimeForm end = chrono::system_clock::now();
		//chrono::milliseconds mill = chrono::duration_cast<chrono::milliseconds>(end - start);		//test 시간 측정						
	}

	//-----------------------------------------------------------------------
	//점 -> 선 만들기
	//-----------------------------------------------------------------------
	tuple_OptimizeY AreaFilling::CreatePointsToLines(tuple_meshInfo& meshInfo, Duint faceNum)
	{		
		//병렬 루프 안에서 사용하기 위해서 아토믹 사용
		atomic<Dfloat> minOptimizeY = 99999.0f;
		atomic<Dfloat> maxOptimizeY = -1.0f;
		vector<Line> vecLine; //병렬화 수준을 끌어올리기 위해서 추가

		const BaseFace& face = get<1>(meshInfo)[faceNum];

		//parallel_for에서는 일반 정수 만 가능하다 즉 unsigned 사용 못한다.---------------------
		//parallel_for(0, (Dint)face.vecIndex.size(), [&](Dint j)
		for( Duint j = 0; j < face.vecIndex.size(); ++j )
		{
			LineKey lineKey;
			lineKey.beginIndex = face.vecIndex[ j ];
					
			//시작 정점
			Vector3 beginVertex = get<0>(meshInfo)[lineKey.beginIndex - 1 ];
			
			if( j == face.vecIndex.size() - 1 )
			{
				lineKey.endIndex = face.vecIndex[ j % (face.vecIndex.size() - 1) ];
			}
			else
			{
				lineKey.endIndex = face.vecIndex[ j + 1 ];
			}
			
			Vector3 endVertex = get<0>(meshInfo)[lineKey.endIndex - 1 ];
						
			//x, y 성분만 올림 ( z는 0~1 사이의 깊이 값 )
			beginVertex.x = ceilf(beginVertex.x);
			beginVertex.y = ceilf(beginVertex.y);
			endVertex.x = ceilf(endVertex.x);
			endVertex.y = ceilf(endVertex.y);

			vecLine.emplace_back(lineKey, beginVertex, endVertex);
								
			Dfloat value = GetSmallYValue(beginVertex, endVertex);
			if (minOptimizeY > value)
			{
				minOptimizeY = value;
			}

			value = GetBigYValue(beginVertex, endVertex);
			if (maxOptimizeY < value)
			{
				maxOptimizeY = value;
			}			
		}

		m_VecLine.swap(vecLine);	//병렬 루프안에서는 m_VecLine이 스레드 세이프하지 않기 때문에 외부에서 스왑
		return make_tuple(minOptimizeY.load(), maxOptimizeY.load());
		//--------------------------------------------------
	}

	//-----------------------------------------------------------------------
	//엣지 테이블 만들기
	//-----------------------------------------------------------------------
	Dvoid AreaFilling::CreateEdgeTable( )
	{
		assert( !m_VecLine.empty());		

		//for(const auto& line : m_VecLine )
		parallel_for_each(begin(m_VecLine), end(m_VecLine), [this](const Line& line)
		{ //이 병렬 루프에서 m_VecLine는 스레드 세이프하지않지만 읽기만 하므로 문제안됨
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
					
			if( line.beginVertex.y < line.endVertex.y )
			{
				node.x_min		= line.beginVertex.x;
				node.y_min		= line.beginVertex.y;	//깊이 보간을 위해서 추가
				node.y_max		= line.endVertex.y;

				node.min_depth	= line.beginVertex.z;	//깊이 보간을 위해서 추가
				node.max_depth  = line.endVertex.z;		//깊이 보간을 위해서 추가
			}
			else
			{
				node.x_min		= line.endVertex.x;
				node.y_min		= line.endVertex.y;		//
				node.y_max		= line.beginVertex.y;

				node.min_depth = line.endVertex.z;		//
				node.max_depth = line.beginVertex.z;	//
			}			

			m_EdgeTable.push_back(LineEdge(line.lineKey, node));
		});
	}

	//-----------------------------------------------------------------------
	//체인 테이블 생성 및 PreProcessing
	//-----------------------------------------------------------------------
	Dvoid AreaFilling::CreateChainTable(tuple_OptimizeY opY)
	{
		assert(!m_VecLine.empty());
		
		int totalEdgeSize = m_VecLine.size();
		list<int> upLineSavelist;	//한 번이라도 윗줄로 올라간 정점들 인덱스 저장

		//parallel_buffered_sort는 리스트에는 적용할수 없다. 연속된 메모리가 아니기 때문에 end와 begin의 메모리 차이를 구할수 없다.
		//m_VecLine.sort([] (const Line& lhs, const Line& rhs) {	return lhs.lineKey.beginIndex < rhs.lineKey.beginIndex;	 });	//인덱스 순으로 오름차순 정렬
		parallel_buffered_sort(begin(m_VecLine), end(m_VecLine), [](const Line& lhs, const Line& rhs) {	return lhs.lineKey.beginIndex < rhs.lineKey.beginIndex;	 });

		//루프마다 m_VecLine이 변화하면서 다음 라인에 영향을 미치므로 병렬화 X
		Dint minY = (Dint)(get<0>(opY));
		Dint maxY = (Dint)(get<1>(opY));

		for (Dint y = minY; y < maxY; ++y)
		{
			list<Line> currentLine;			

			for( auto& lineIter = m_VecLine.begin(); lineIter != m_VecLine.end(); )
			{			
				ITER_CONVERT(pLine, &(*lineIter));
				if (y == pLine->beginVertex.y || y == pLine->endVertex.y) //해당 y축에 정점이 걸치는지 검사			 	
				{
					currentLine.emplace_back( *pLine );
					lineIter = m_VecLine.erase( lineIter );
				}
				else
				{
					++lineIter;
				}
			} // 현재 라인에 포함가능한 엣지들 모두 포함

			for( auto& chainIter = currentLine.begin(); chainIter != currentLine.end(); )
			{
				Dbool isOnceCount = true;
				for( auto& innerIter : currentLine )
				{
					if( chainIter->lineKey == innerIter.lineKey )	//같으면 리턴
					{
						continue;
					}

					if( chainIter->lineKey.beginIndex % totalEdgeSize > innerIter.lineKey.beginIndex % totalEdgeSize )	 //인덱스로 그려진 순서 비교
					{
						isOnceCount = false;
						continue;
					}

					if( CheckContinueLine( chainIter->lineKey, innerIter.lineKey) )	 //서로 연결되어있으면 일반적인 경우
					{
						isOnceCount = false;
						break;
					}
					isOnceCount = true;
				}

				if( isOnceCount )
				{
					if( find(upLineSavelist.begin(), upLineSavelist.end(), chainIter->lineKey.beginIndex) == upLineSavelist.end() )	//리스트에 포함되지 않았다 즉, 올라간적 없다.
					{						
						upLineSavelist.push_back( chainIter->lineKey.beginIndex );	//한번 올라간 내역 저장
						++(chainIter->GetMinY());			//y축 값 1올리기								
						m_VecLine.emplace_back( *chainIter );//다시 검색하기 위해서 리스트에 넣어두자						
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

		assert(!m_ActiveTable.empty());		
	}
	
	//-------------------------------------------------------------------
	//해당 위치 점 찍기
	//-------------------------------------------------------------------
	Dvoid AreaFilling::DrawDot(const Duint x, const Duint y, const Dulong DotColor)
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
	Dvoid AreaFilling::DrawLine(vector<EdgeNode>& renderLine, const Dint currentHeight, const Dulong dotColor)
	{
		Dint beginX = -1;
		Dint endX = -1;
		Dint odd_even = 0;
		Dfloat DepthLeft, DepthRight;

		//순서대로 진행해야함 병렬 X
		for( auto& dotNode : renderLine )
		{			
			if( odd_even % 2 == 0 ) //짝수
			{
				beginX = (int)ceilf(dotNode.x_min); //무조건 올림				
				
				//좌측 위치에서의 깊이값 보간
				Dfloat dy = dotNode.y_max - dotNode.y_min;
				Dfloat dz = dotNode.max_depth - dotNode.min_depth;
				Dfloat rate = (currentHeight - dotNode.y_min )/ dy;
				DepthLeft = dotNode.min_depth + ( dz * rate );
			}
			else //홀수
			{
				endX = (int)ceilf(dotNode.x_min);
				//우측 위치에서의 깊이값 보간
				Dfloat dy = dotNode.y_max - dotNode.y_min;
				Dfloat dz = dotNode.max_depth - dotNode.min_depth;
				Dfloat rate = (currentHeight - dotNode.y_min) / dy;
				DepthRight = dotNode.min_depth + (dz * rate);


				//for( Dint i = beginX; i < endX; ++i )
				parallel_for(beginX, endX, 1, [&]( Dint i )
				{
					//좌, 우 사이의 깊이값 보간
					Dfloat dd = DepthRight - DepthLeft;
					Dfloat dx = (Dfloat)endX - beginX;
					Dfloat rate = (i - beginX) / dx;
					Dfloat resultDepth = DepthLeft + (dd * rate);

					if( DepthTest(i, currentHeight, resultDepth) )
					{
						DrawDot(i, currentHeight, dotColor);
					}					
				});
			}

			dotNode.x_min += dotNode.reverseSlope;
			++odd_even;
		}
	}	
	
	//-------------------------------------------------------------------
	// 메쉬의 해당 면 그리기
	//-------------------------------------------------------------------
	Dvoid AreaFilling::DrawFace(const Dulong dotColor)
	{		
		assert(m_Buffer != nullptr && !m_ActiveTable.empty());

		vector<EdgeNode> continueRenderLine;	//이 컨테이너는 erase를 사용해야하기 때문에 병렬컨테이너로 하면 안된다.
		auto& activeIter = m_ActiveTable.begin();

		//m_ActiveTable.sort([] (const ActiveLine& lhs, const ActiveLine& rhs) -> Dbool { return lhs.height < rhs.height; } );
		parallel_buffered_sort(begin(m_ActiveTable), end(m_ActiveTable), [](const ActiveLine& lhs, const ActiveLine& rhs) -> Dbool { return lhs.height < rhs.height; });
		for (Dint y = activeIter->height;; ++y)	//한줄한줄 그려나가야 하기 때문에 ActiveTable에서 시작값의 높이로 초기화
		{
			if( y == activeIter->height )	//라인 정점과 높이가 겹치는 부분
			{
				list<Line>& currentLine = activeIter->currentLine;
				//parallel_for_each(begin(currentLine), end(currentLine), [&](const Line& line)	//continueRenderLine이 스레드 세이프 하지 않기 떄문에 사용X
				for( auto& line : activeIter->currentLine ) //현재 높이에 걸쳐진 라인들을 순회				
				{
					auto& etIter = STD_FIND_IF(m_EdgeTable, [&line] (const LineEdge& le) { return le.lineKey == line.lineKey; });
					if( etIter != m_EdgeTable.end() )
					{
						continueRenderLine.emplace_back(etIter->edgeNode);
					}
				}
				
				//continueRenderLine.sort([](const EdgeNode& lhs, const EdgeNode& rhs)
				parallel_buffered_sort(begin(continueRenderLine), end(continueRenderLine), [](const EdgeNode& lhs, const EdgeNode& rhs)
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
				Clear();
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
	Dbool AreaFilling::CheckContinueLine(const LineKey& lhs, const LineKey& rhs) const
	{	
		if( lhs.beginIndex	== rhs.endIndex ||			
			lhs.endIndex	== rhs.beginIndex )
		{
			return true;
		}

		return false;
	}	

	//-------------------------------------------------------------------
	//y축 정점이 일렬로 있을경우 체크 (즉, 삼각형을 이루고 있지 않을 때)
	//-------------------------------------------------------------------
	Dbool AreaFilling::CheckYStandInLine() const
	{
		if ( m_VecLine.size() )
		{
			Dfloat y = m_VecLine[0].beginVertex.y;			
			for (auto& line : m_VecLine)
			{
				if (y != line.endVertex.y)
				{
					return false;
				}				
			}
			return true;			
		}
		
		return true; //y축이 일치한다 즉 그려져선 안된다.
	}

	void AreaFilling::SetTransform(TransType type, const Matrix44 matrix)
	{
		m_arrayTransform[ type ] = matrix;
	}

	void AreaFilling::SetScreenInfo(Dvoid* buffer, const int width, const int height)
	{
		m_Buffer = (Duchar*)buffer;
		m_Width = width;
		m_Height = height;
		
		m_DepthBuffer = new Dfloat[ width * height ];
		for( int i = 0; i < height; ++i )
		{
			for( int j = 0; j < width; ++j )
			{
				m_DepthBuffer[ i * width + j] = 1.0f;
			}
		}		
	}

	const array<Matrix44, TRANSFORM_END>& AreaFilling::GetArrayTransform()
	{
		return m_arrayTransform;
	}

	Dbool AreaFilling::DepthTest(const Duint x, const Duint y, Dfloat depth)
	{
		if( m_DepthBuffer[ y * m_Width + x ] > depth )
		{
			m_DepthBuffer[ y * m_Width + x ] = depth;
			return true;
		}

		return false;
	}	

	Dvoid AreaFilling::Clear()
	{
		m_ActiveTable.clear();
		m_EdgeTable.clear();
		m_VecLine.clear();
	}	

	Dfloat AreaFilling::GetBigYValue(const Vector3& begin, const Vector3& end) const
	{
		return (begin.y < end.y) ? end.y : begin.y;
	}

	Dfloat AreaFilling::GetSmallYValue(const Vector3& begin, const Vector3& end) const
	{
		return (begin.y < end.y) ? begin.y : end.y;
	}

}