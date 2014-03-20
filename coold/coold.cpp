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

CoolD::CustomMesh* g_pMesh = nullptr;	//�ϴ� �޽��� 1���� �ٷ�ٴ� ������ ����....
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
		
	//�� -> �� �����---------------------------------------------------
	//for( Duint i = 1; i < g_pMesh->GetFaceSize(); ++i )
	{
		const baseFace& face = g_pMesh->GetFace( 1 );		//�ϴ� 1�� �ﰢ���� �׸��°ɷ� ����
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

	//���� ���̺� ���� �� PreProcessing
	//-----------------------------------------------------------------
	int totalEdgeSize = listLine.size();
	list<int> upLineSavelist;	//�� ���̶� ���ٷ� �ö� ������ �ε��� ����
	for( int y = 0; y < height; ++y)
	{
		list<Line*> chainLine;
				
		//for( auto& line : listLine )
		listLine.sort( [](const Line* lhs, const Line* rhs) {	return lhs->beginIndex < rhs->beginIndex;	 } );	//�ε��� ������ �������� ����
		
		for(auto& lineIter = listLine.begin(); lineIter != listLine.end(); )
		{			
			if( y == (*lineIter)->beginVertex.y || y == (*lineIter)->endVertex.y ) //�ش� y�࿡ ������ ��ġ���� �˻�			 	
			{	
				chainLine.push_back( *lineIter );
				lineIter = listLine.erase(lineIter);
			}
			else
			{
				++lineIter;
			}
		} // ���� ���ο� ���԰����� ������ ��� ����

		for( auto& iter = chainLine.begin(); iter != chainLine.end(); )		
		{			
			bool isOnceCount = true;
			for( auto& innerIter : chainLine )
			{					
				if( (*iter) == innerIter )	//������ ����
				{
					continue;
				}
					
				if( (*iter)->beginIndex % totalEdgeSize > innerIter->beginIndex % totalEdgeSize )	 //�ε����� �׷��� ���� ��
				{
					isOnceCount = false;
					continue;
				}
				
				if( (*iter)->beginIndex == innerIter->endIndex || (*iter)->endIndex == innerIter->beginIndex )	 //���� ����Ǿ������� �Ϲ����� ���
				{
					isOnceCount = false;
					break;
				}
				isOnceCount = true;
			}

			if( isOnceCount )
			{
				if( find(upLineSavelist.begin(), upLineSavelist.end(), (*iter)->beginIndex ) == upLineSavelist.end() )	//����Ʈ�� ���Ե��� �ʾҴ� ��, �ö��� ����.
				{
					upLineSavelist.push_back((*iter)->beginIndex	);	//�ѹ� �ö� ���� ����
					((*iter)->beginVertex.y < (*iter)->endVertex.y)? (++(*iter)->beginVertex.y) : (++(*iter)->endVertex.y);		
					listLine.push_back( *iter );	//�ٽ� �˻��ϱ� ���ؼ� ����Ʈ�� �־����
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

