#include "CoolD_AreaFilling.h"
#include "..\Data\CoolD_Inlines.h"
#include "CoolD_CustomMesh.h"
#include "CoolD_Transform.h"

namespace CoolD
{
	AreaFilling::AreaFilling()
		: m_Buffer(nullptr), m_DepthBuffer(nullptr), m_CullMode(BSCULL::CCW)
	{			
		m_vecLine.resize(100);
		m_edgeTable.resize(100);
		m_activeTable.resize(100);
	}

	AreaFilling::~AreaFilling()
	{					
		Safe_Delete_Array(m_DepthBuffer);
	}

	Dvoid AreaFilling::ClearColorBuffer(Dulong clearColor)
	{		
		Dint totalPixelCount = m_Height * m_Width * 3;
				
		for( Dint i = 0; i < totalPixelCount; i+=3)
		{		
			m_Buffer[ i ] = (Duchar)(clearColor >> 24);
			m_Buffer[ i + 1 ] = (Duchar)(clearColor >> 16);
			m_Buffer[ i + 2 ] = (Duchar)(clearColor >> 8);
		}
		
		Dint totalDepthClearCount = m_Height * m_Width;
		if( m_DepthBuffer == nullptr )
		{
			m_DepthBuffer = new Dfloat[ totalDepthClearCount ];
		}
		
		memset(m_DepthBuffer, 0, totalDepthClearCount * sizeof(Dfloat));		//0으로 초기화 하고 깊이 테스트할때는 1 - 깊이 값으로 비교한다.
	}

	Dvoid AreaFilling::Render(const vector<Vector3>* pvecVertex, const vector<BaseFace>* pvecFace)
	{							
		for( Dint faceNum = 0; faceNum < (Dint)pvecFace->size(); ++faceNum )
		{
			m_vecLine.clear();
			m_edgeTable.clear();
			m_activeTable.clear();

			CreatePointsToLines(pvecVertex, pvecFace, faceNum, m_vecLine);

			//backspace culling
			if (!BackSpaceCulling(m_vecLine))
			{
				continue;
			}

			auto unusualValue = STD_FIND_IF(m_vecLine, [] (const Line& line)
			{	//모든 요소의 y 값이 같은지 체크
				return line.beginVertex.y != line.endVertex.y;
			});

			if( unusualValue != end(m_vecLine) )
			{
				CreateEdgeTable(m_vecLine, m_edgeTable);
				CreateChainTable(m_vecLine, m_activeTable);
				DrawFace(m_activeTable, m_edgeTable, MixDotColor((*pvecFace)[faceNum].color));
			}
		}

		//combinable<vector<Line>> vecLine;
		//combinable<vector<LineEdge>> edgeTable;
		//combinable<vector<ActiveLine>> activeTable;
			
		///*vecLine.local().reserve(pvecVertex->size());
		//edgeTable.local().reserve(10000);
		//activeTable.local().reserve(20);*/

		//parallel_for(0, (Dint)pvecFace->size(), [&] (Dint faceNum)		
		//{
		//	vecLine.local().clear();
		//	edgeTable.local().clear();
		//	activeTable.local().clear();
		//			
		//	CreatePointsToLines(pvecVertex, pvecFace, faceNum, vecLine.local());
		//	
		//	auto& unusualValue = STD_FIND_IF(vecLine.local(), [](const Line& line)
		//	{	//모든 요소의 y 값이 같은지 체크
		//		return line.beginVertex.y != line.endVertex.y;
		//	});

		//	if( unusualValue != end(vecLine.local()) )
		//	{		
		//		CreateEdgeTable(vecLine.local(), edgeTable.local());
		//		CreateChainTable(vecLine.local(), activeTable.local());
		//		DrawFace(activeTable.local(), edgeTable.local(), MixDotColor((*pvecFace)[ faceNum ].color));
		//	}			
		//});
	}

	//-----------------------------------------------------------------------
	//점 -> 선 만들기
	//-----------------------------------------------------------------------
	Dvoid AreaFilling::CreatePointsToLines(const vector<Vector3>* pvecVertex, const vector<BaseFace>* pvecFace, Duint faceNum, vector<Line>& vecLine)
	{
		const BaseFace& face = (*pvecFace)[ faceNum ];

		Duint faceSize = face.vecIndex.size();

		LineKey lineKey;
		lineKey.beginIndex = face.vecIndex[ faceSize - 1 ];
		lineKey.endIndex = face.vecIndex[ 0 ];			

		Vector3 beginVertex = (*pvecVertex)[ lineKey.beginIndex - 1 ];
		Vector3 endVertex = (*pvecVertex)[ lineKey.endIndex - 1 ];

		InsertPointToLine(vecLine, lineKey, beginVertex, endVertex);

		for( Duint j = 0; j < faceSize - 1; ++j )
		{
			lineKey.beginIndex = face.vecIndex[ j ];
			lineKey.endIndex = face.vecIndex[ j + 1 ];			

			beginVertex = (*pvecVertex)[ lineKey.beginIndex - 1 ];
			endVertex = (*pvecVertex)[ lineKey.endIndex - 1 ];

			InsertPointToLine(vecLine, lineKey, beginVertex, endVertex);
		}
	}
	//-----------------------------------------------------------------------
	//엣지 테이블 만들기
	//-----------------------------------------------------------------------
	Dvoid AreaFilling::CreateEdgeTable(vector<Line>& vecLine, vector<LineEdge>& edgeTable)
	{
		assert(!vecLine.empty());

		EdgeNode node;
		Dfloat dx, dy;
		for (const auto& line : vecLine)
		{ 
			dx = line.endVertex.x - line.beginVertex.x;
			dy = line.endVertex.y - line.beginVertex.y;			
			
			if( dx == 0 || dy == 0 )
			{	//분모가 0이면 무조건 0 ZeroDivide방지
				node.reverseSlope = 0;
			}
			else
			{
				node.reverseSlope = dx / dy;
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

			edgeTable.push_back(LineEdge(line.lineKey, node));
		}
	}

	//-----------------------------------------------------------------------
	//체인 테이블 생성 및 PreProcessing
	//-----------------------------------------------------------------------
	Dvoid AreaFilling::CreateChainTable(vector<Line>& vecLine, vector<ActiveLine>& activeTable)
	{
		assert(!vecLine.empty());
		vector<Dint> vecUpLineSave;	//한 번이라도 윗줄로 올라간 정점들 인덱스 저장
		
		auto maxIter = max_element(begin(vecLine), end(vecLine), [](const Line& lhs, const Line& rhs)	{	return lhs.beginVertex.y < rhs.beginVertex.y; 	});
		auto minIter = min_element(begin(vecLine), end(vecLine), [](const Line& lhs, const Line& rhs)	{	return lhs.beginVertex.y < rhs.beginVertex.y;	});

		Dint maxY = (Dint)maxIter->beginVertex.y;
		Dint minY = (Dint)minIter->beginVertex.y;

		vector<Line> currentLine;
		currentLine.reserve(3);

		for (Dint y = minY; y < maxY; ++y)
		{				
			currentLine.clear();
			for (auto lineIter = vecLine.begin(); lineIter != vecLine.end();)
			{
				ITER_CONVERT(pLine, &(*lineIter));
				if (y == pLine->beginVertex.y || y == pLine->endVertex.y) //해당 y축에 정점이 걸치는지 검사			 	
				{
					currentLine.emplace_back( *pLine );
					lineIter = vecLine.erase(lineIter);
				}
				else
				{
					++lineIter;
				}
			} // 현재 라인에 포함가능한 엣지들 모두 포함
						
			if( currentLine.empty() )
			{
				continue;
			}		
			
			for( auto chainIter = currentLine.begin(); chainIter != currentLine.end(); )
			{
				auto retIter = STD_FIND_IF(currentLine, [&] (const Line& line){	return CheckContinueLine(chainIter->lineKey, line.lineKey);		});
				if( retIter == currentLine.end() )	//특정 정점이 연결되어 있지 않다 ( 1번만 카운트 됨 14_ScanLineFill.pdf 참조 )
				{
					if( find(vecUpLineSave.begin(), vecUpLineSave.end(), chainIter->lineKey.beginIndex) == vecUpLineSave.end() )	//리스트에 포함되지 않았다 즉, 올라간적 없다.
					{
						vecUpLineSave.push_back(chainIter->lineKey.beginIndex);	//한번 올라간 내역 저장
						++(chainIter->GetMinY());		 //y축 값 1올리기								
						vecLine.emplace_back(*chainIter);//다시 검색하기 위해서 리스트에 넣어두자						
						chainIter = currentLine.erase(chainIter);
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
						
			if( !currentLine.empty() )
			{
				STD_ERASE(currentLine, STD_REMOVE_IF(currentLine, [] (const Line& l){ return l.beginVertex.y == l.endVertex.y; }));
				activeTable.emplace_back(y, currentLine);
			}
		}	
	}

	Dbool AreaFilling::BackSpaceCulling( vector<Line>& vecLine )
	{
		if( vecLine.size() > 2 )
		{	
			Vector3 v1(0, 0, -1);
			Vector3 v2 = (vecLine[ 0 ].endVertex - vecLine[ 0 ].beginVertex).Cross(vecLine[ 1 ].endVertex - vecLine[ 1 ].beginVertex);
			
			v2.Normalize();
			switch( m_CullMode )
			{
			case BSCULL::CW:
				if( v1.Dot(v2) <= 0 )
				{
					return true;
				}
				break;
			case BSCULL::CCW:
				if( v1.Dot(v2) >= 0 )
				{
					return true;
				}
				break;				
			case BSCULL::ALL:
				return true;				
			}			
		}

		return false;
	}

	//-------------------------------------------------------------------
	//해당 위치 점 찍기
	//-------------------------------------------------------------------
	Dvoid AreaFilling::DrawDot(const Duint& x, const Duint& y, const Dulong& DotColor)
	{	
		Dint index = (((m_Width * y) + x) * 3);

		m_Buffer[ index ] = (Duchar)(DotColor >> 24);
		m_Buffer[ index + 1 ] = (Duchar)(DotColor >> 16);
		m_Buffer[ index + 2 ] = (Duchar)(DotColor >> 8);
	}

	//-------------------------------------------------------------------
	//현재 라인의 노드 사이 점 찍기
	//-------------------------------------------------------------------
	Dvoid AreaFilling::DrawLine(vector<EdgeNode>& renderLine, const Dint currentHeight, const Dulong dotColor)
	{
		Dfloat beginX = -1.0f;
		Dfloat endX = -1.0f;
		Dfloat DepthLeft, DepthRight;

		vector<EdgeNode>::iterator curruntIter;
		vector<EdgeNode>::iterator nextIter;
		for( auto iter = begin(renderLine);; ++iter )
		{
			curruntIter = iter;
			if( curruntIter == end(renderLine) )
				break;
						
			nextIter = (++iter);
			if(  nextIter == end(renderLine) )
				break;

			beginX = curruntIter->x_min;
			DepthLeft = GetDepthInterpolation(curruntIter->y_max, curruntIter->y_min, curruntIter->max_depth, curruntIter->min_depth, currentHeight);

			endX = nextIter->x_min;
			DepthRight = GetDepthInterpolation(nextIter->y_max, nextIter->y_min, nextIter->max_depth, nextIter->min_depth, currentHeight);

			curruntIter->x_min += curruntIter->reverseSlope;
			nextIter->x_min += nextIter->reverseSlope;
						
			for( Dint i = (Dint)beginX; i < (Dint)endX; ++i )				
			{
				if (i < 0 || i >= m_Width || currentHeight < 0 || currentHeight >= m_Height)
				{
					continue;
				}
				//좌, 우 사이의 깊이값 보간					
				Dfloat resultDepth = GetDepthInterpolation(endX, beginX, DepthRight, DepthLeft, i);

				if( DepthTest(i, currentHeight, resultDepth) )
				{
					DrawDot(i, currentHeight, dotColor);
				}
			}			
		}		
	}	
	
	//-------------------------------------------------------------------
	// 메쉬의 해당 면 그리기
	//-------------------------------------------------------------------
	Dvoid AreaFilling::DrawFace(vector<ActiveLine>& activeTable, vector<LineEdge>& edgeTable, const Dulong dotColor)
	{		
		assert(m_Buffer != nullptr && !activeTable.empty());

		vector<EdgeNode> continueRenderLine;
		auto activeIter = activeTable.begin();
		
		for (Dint y = activeIter->height;; ++y)	//한줄한줄 그려나가야 하기 때문에 ActiveTable에서 시작값의 높이로 초기화
		{
			if( y == activeIter->height )	//라인 정점과 높이가 겹치는 부분
			{
				for( auto& line : activeIter->currentLine ) //현재 높이에 걸쳐진 라인들을 순회				
				{
					auto iter = STD_FIND_IF(edgeTable, [&line](const LineEdge& le) { return le.lineKey == line.lineKey; });
					if( iter != edgeTable.end() )
					{
						continueRenderLine.emplace_back(iter->edgeNode);
					}
				}				
									
				sort(begin(continueRenderLine), end(continueRenderLine), [](const EdgeNode& lhs, const EdgeNode& rhs)
				{
					if (lhs.x_min == rhs.x_min)
					{	//기울기가 0인 경우가 있기 때문에 기울기 값만으로는 순서를 결정할수 없어서 x_min과 더한 결과값으로 비교한다.
						return lhs.x_min + lhs.reverseSlope < rhs.x_min + rhs.reverseSlope;
					}
					return lhs.x_min < rhs.x_min;
				});	//x_min값 크기 순으로 오름차순 정렬
				
				if (activeIter != (--activeTable.end()))	//검사 대상을 다음번 위 정점으로 이동
				{
					++activeIter;
				}
			}

			DrawLine(continueRenderLine, y, dotColor);	//현재 라인 도트 찍기

			if( continueRenderLine.empty() )
			{
				break;				
			}
			else
			{
				STD_ERASE(continueRenderLine, STD_REMOVE_IF(continueRenderLine, [y] (const EdgeNode& node){return node.y_max == y; }));
			}			
		}
	}

	void AreaFilling::SetScreenInfo(Dvoid* buffer, const int width, const int height)
	{
		if( m_Buffer != buffer )
		{
			m_Buffer = (Duchar*)buffer;
			m_Width = width;
			m_Height = height;
		}		
	}	

	Dbool AreaFilling::DepthTest(const Duint x, const Duint y, Dfloat depth)
	{		
		Dfloat* buffer = &m_DepthBuffer[ y * m_Width + x ];
		Dfloat opDepth = 1.0f - depth;

		if( *buffer < opDepth )
		{
			*buffer = opDepth;
			return true;
		}

		return false;
	}		

	Dvoid AreaFilling::InsertPointToLine(vector<Line>& vecLine, LineKey& lineKey, Vector3& beginVertex, Vector3& endVertex)
	{			
		beginVertex.x = floor(beginVertex.x);
		beginVertex.y = floor(beginVertex.y);
		endVertex.x = floor(endVertex.x);
		endVertex.y = floor(endVertex.y);

		vecLine.emplace_back(lineKey, beginVertex, endVertex);
	}	
}