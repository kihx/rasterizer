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
map<Dint, list<Line*> > chainTable;

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
	ClearColorBuffer(buffer, width, height, RED );
		
	//점 -> 선 만들기---------------------------------------------------
	//for( Duint i = 1; i < g_pMesh->GetFaceSize(); ++i )
	{
		const baseFace& face = g_pMesh->GetFace( 1 );		//일단 1개 삼각형만 그리는걸로 진행
		for( Duint j = 0; j < face.vecIndex.size() ; ++j )
		{
			Line* line = new Line();			
			line->beginIndex = face.vecIndex[j];
			line->beginVertex = g_pMesh->GetVertex( line->beginIndex );
						
			if( j == face.vecIndex.size() - 1 )
			{				
				line->endIndex	= face.vecIndex[ j % (face.vecIndex.size() -1) ];				
			}
			else
			{
				line->endIndex	= face.vecIndex[ j + 1 ];				
			}	
			line->endVertex  = g_pMesh->GetVertex( line->endIndex ); 
			listLine.push_back(line);			
		}
	}

	//엣지 테이블 생성 및 PreProcessing
	//-----------------------------------------------------------------
	int totalEdgeSize = listLine.size();
	list<int> upLineSavelist;	//한 번이라도 윗줄로 올라간 정점들 인덱스 저장
	for( int y = 0; y < height; ++y)
	{
		list<Line*> chainLine;
				
		//for( auto& line : listLine )
		listLine.sort( [](const Line* lhs, const Line* rhs) {	return lhs->beginIndex < rhs->beginIndex;	 } );	//인덱스 순으로 오름차순 정렬
		
		for(auto& lineIter = listLine.begin(); lineIter != listLine.end(); )
		{			
			if( y == (*lineIter)->beginVertex.y || y == (*lineIter)->endVertex.y ) //해당 y축에 정점이 걸치는지 검사			 	
			{	
				chainLine.push_back( *lineIter );
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
					
				if( (*iter)->beginIndex % totalEdgeSize > innerIter->beginIndex % totalEdgeSize )	 //인덱스로 그려진 순서 비교
				{
					isOnceCount = false;
					continue;
				}
				
				if( (*iter)->beginIndex == innerIter->endIndex || (*iter)->endIndex == innerIter->beginIndex )	 //서로 연결되어있으면 일반적인 경우
				{
					isOnceCount = false;
					break;
				}
				isOnceCount = true;
			}

			if( isOnceCount )
			{
				if( find(upLineSavelist.begin(), upLineSavelist.end(), (*iter)->beginIndex ) == upLineSavelist.end() )	//리스트에 포함되지 않았다 즉, 올라간적 없다.
				{
					upLineSavelist.push_back((*iter)->beginIndex	);	//한번 올라간 내역 저장
					((*iter)->beginVertex.y < (*iter)->endVertex.y)? (++(*iter)->beginVertex.y) : (++(*iter)->endVertex.y);		
					listLine.push_back( *iter );	//다시 검색하기 위해서 리스트에 넣어두자
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

		chainLine.erase( remove_if( chainLine.begin(), chainLine.end(), []( const Line* l ){ return l->beginVertex.y == l->endVertex.y; } ), chainLine.end());
		
		if( !chainLine.empty() )
		{
			chainTable.insert( pair<Dint, list<Line*> >( y, chainLine ) );
		}		
	}
}

