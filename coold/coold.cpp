#define DLLEXPORTS
//#include "Windows.h"
//#include <memory>
#include "coold.h"
#include "CoolD_CustomMesh.h"
#include "CoolD_SubFunction.h"
#include "CoolD_Defines.h"
#include "CoolD_Inlines.h"


//BOOL WINAPI DllMain( HINSTANCE hInstDll, DWORD fdwReason, LPVOID fImpLoad )
//{
//	return TRUE;
//}

list<Line*> listLine;
map<Dint, list<Line*>> chainTable;
map<LineKey, EdgeNode*> edgeTable;

CoolD::CustomMesh* g_pMesh = nullptr;	//일단 메쉬는 1개만 다룬다는 전제로 진행....
EXTERN_FORM DLL_API void __cdecl coold_LoadMeshFromFile( const Dchar* filename )
{
	if( g_pMesh )
	{
		Safe_Delete( g_pMesh );
	}
	
	g_pMesh = CoolD::CustomMesh::CreateMeshFromFile( filename );
}

EXTERN_FORM DLL_API void __cdecl coold_RenderToBuffer( void* buffer, Dint width, Dint height, Dint bpp )
{
	ClearColorBuffer(buffer, width, height, BLACK );
		
	//-----------------------------------------------------------------------
	//점 -> 선 만들기
	//-----------------------------------------------------------------------
	for( Duint faceNum = 1; faceNum <= g_pMesh->GetFaceSize(); ++faceNum )
	{
		const BaseFace& face = g_pMesh->GetFace(faceNum);		//일단 1개 삼각형만 그리는걸로 진행

		for( Duint j = 0; j < face.vecIndex.size(); ++j )
		{
			Line* line = new Line();
			line->lineKey.beginIndex = face.vecIndex[ j ];
			line->beginVertex = g_pMesh->GetVertex(line->lineKey.beginIndex);

			if( j == face.vecIndex.size() - 1 )
			{
				line->lineKey.endIndex = face.vecIndex[ j % (face.vecIndex.size() - 1) ];
			}
			else
			{
				line->lineKey.endIndex = face.vecIndex[ j + 1 ];
			}
			line->endVertex = g_pMesh->GetVertex(line->lineKey.endIndex);
			listLine.push_back(line);
		}

		//-----------------------------------------------------------------------
		//엣지 테이블 만들기
		//-----------------------------------------------------------------------
		for( auto line : listLine )
		{
			EdgeNode* node = new EdgeNode();

			Dfloat dx = line->endVertex.x - line->beginVertex.x;
			Dfloat dy = line->endVertex.y - line->beginVertex.y;

			if( dx == 0 || dy == 0 )
			{	//분모가 0이면 무조건 0 ZeroDivide방지
				node->reverseSlope = 0;
			}
			else
			{
				node->reverseSlope = dx / dy; // dx/dy가 기울기이고 그 역수값을 저장해야하기 때문에
			}

			node->x_min = (line->beginVertex.y < line->endVertex.y) ? line->beginVertex.x : line->endVertex.x;
			node->y_max = (line->beginVertex.y < line->endVertex.y) ? line->endVertex.y : line->beginVertex.y;

			edgeTable.insert(pair<LineKey, EdgeNode*>(line->lineKey, node));
		}

		//-----------------------------------------------------------------------
		//체인 테이블 생성 및 PreProcessing
		//-----------------------------------------------------------------------
		int totalEdgeSize = listLine.size();
		list<int> upLineSavelist;	//한 번이라도 윗줄로 올라간 정점들 인덱스 저장
		for( int y = 0; y < height; ++y )
		{
			list<Line*> chainLine;

			//for( auto& line : listLine )
			listLine.sort([] (const Line* lhs, const Line* rhs) {	return lhs->lineKey.beginIndex < rhs->lineKey.beginIndex;	 });	//인덱스 순으로 오름차순 정렬

			for( auto& lineIter = listLine.begin(); lineIter != listLine.end(); )
			{
				if( y == (*lineIter)->beginVertex.y || y == (*lineIter)->endVertex.y ) //해당 y축에 정점이 걸치는지 검사			 	
				{
					chainLine.push_back(*lineIter);
					lineIter = listLine.erase(lineIter);
				}
				else
				{
					++lineIter;
				}
			} // 현재 라인에 포함가능한 엣지들 모두 포함

			for( auto& iter = chainLine.begin(); iter != chainLine.end(); )
			{
				bool isOnceCount = true;
				for( auto& innerIter : chainLine )
				{
					if( (*iter) == innerIter )	//같으면 리턴
					{
						continue;
					}

					if( (*iter)->lineKey.beginIndex % totalEdgeSize > innerIter->lineKey.beginIndex % totalEdgeSize )	 //인덱스로 그려진 순서 비교
					{
						isOnceCount = false;
						continue;
					}

					if( (*iter)->lineKey.beginIndex == innerIter->lineKey.endIndex ||
						(*iter)->lineKey.endIndex == innerIter->lineKey.beginIndex )	 //서로 연결되어있으면 일반적인 경우
					{
						isOnceCount = false;
						break;
					}
					isOnceCount = true;
				}

				if( isOnceCount )
				{
					if( find(upLineSavelist.begin(), upLineSavelist.end(), (*iter)->lineKey.beginIndex) == upLineSavelist.end() )	//리스트에 포함되지 않았다 즉, 올라간적 없다.
					{
						upLineSavelist.push_back((*iter)->lineKey.beginIndex);	//한번 올라간 내역 저장
						((*iter)->beginVertex.y < (*iter)->endVertex.y) ? (++(*iter)->beginVertex.y) : (++(*iter)->endVertex.y);
						listLine.push_back(*iter);	//다시 검색하기 위해서 리스트에 넣어두자
						iter = chainLine.erase(iter);
					}
					else
					{
						++iter;
					}
				}
				else
				{
					++iter;
				}
			}

			chainLine.erase(remove_if(chainLine.begin(), chainLine.end(), [] (const Line* l){ return l->beginVertex.y == l->endVertex.y; }), chainLine.end());

			if( !chainLine.empty() )
			{
				chainTable.insert(pair<Dint, list<Line*> >(y, chainLine));
			}
		}
		//-------------------------------------------------------------------
		// 렌더링
		//-------------------------------------------------------------------
		list<EdgeNode*> continueRenderLine;
		auto& chainIter = chainTable.begin();
		for( Dint y = (*chainTable.begin()).first;; ++y )	//한줄한줄 그려나가야 하기 때문에 
		{
			if( y == chainIter->first )	//라인 정점과 높이가 겹치는 부분
			{
				for( auto& chainLine : chainIter->second ) //현재 높이에 걸쳐진 라인들을 순회
				{
					EdgeNode* node = (*edgeTable.find(chainLine->lineKey)).second;
					continueRenderLine.push_back(node);
				}

				continueRenderLine.sort([] (const EdgeNode* lhs, const EdgeNode* rhs)
				{
					if( lhs->x_min == rhs->x_min )
					{	//기울기가 0인 경우가 있기 때문에 기울기 값만으로는 순서를 결정할수 없어서 x_min과 더한 결과값으로 비교한다.
						return lhs->x_min + lhs->reverseSlope < rhs->x_min + rhs->reverseSlope;
					}
					return lhs->x_min < rhs->x_min;
				});	//x_min값 크기 순으로 오름차순 정렬

				if( chainIter != (--chainTable.end()) )	//검사 대상을 다음번 위 정점으로 이동
				{
					++chainIter;
				}
			}

			//노드 사이점 찍기----------------		
			Dint beginX = -1;
			Dint endX = -1;
			Dint odd_even = 0;
			for( auto& dotNode : continueRenderLine )
			{
				dotNode->x_min += dotNode->reverseSlope;

				if( odd_even % 2 == 0 ) //짝수
				{
					beginX = (int)ceilf(dotNode->x_min); //무조건 올림				
				}
				else //홀수			
				{
					endX = (int)ceilf(dotNode->x_min);

					for( Dint i = beginX; i <= endX; ++i )
					{
						DotRender(buffer, width, height, i, y, MixDotColor(face.color));
					}
				}
				++odd_even;
			}

			continueRenderLine.erase(remove_if(continueRenderLine.begin(), continueRenderLine.end(),
				[y] (const EdgeNode* node)	{	return node->y_max == y; }),
				continueRenderLine.end());	//y_max값이 현재 y라인과 같다면 리스트에서 삭제

			//--------------------------------
			if( continueRenderLine.size() == 0 )	//한면이 그려지는 시점에는 항상 이 리스트에 값이 존재한다. 없을경우는 끝난경우
			{
				Safe_Delete_ListInMap(chainTable);
				Safe_Delete_Map(edgeTable);
				break;
			}
		}
	}
}

