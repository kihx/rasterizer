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
	ClearColorBuffer(buffer, width, height, BLACK );
		
	//-----------------------------------------------------------------------
	//�� -> �� �����
	//-----------------------------------------------------------------------
	for( Duint faceNum = 1; faceNum <= g_pMesh->GetFaceSize(); ++faceNum )
	{
		const BaseFace& face = g_pMesh->GetFace(faceNum);		//�ϴ� 1�� �ﰢ���� �׸��°ɷ� ����

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
		//���� ���̺� �����
		//-----------------------------------------------------------------------
		for( auto line : listLine )
		{
			EdgeNode* node = new EdgeNode();

			Dfloat dx = line->endVertex.x - line->beginVertex.x;
			Dfloat dy = line->endVertex.y - line->beginVertex.y;

			if( dx == 0 || dy == 0 )
			{	//�и� 0�̸� ������ 0 ZeroDivide����
				node->reverseSlope = 0;
			}
			else
			{
				node->reverseSlope = dx / dy; // dx/dy�� �����̰� �� �������� �����ؾ��ϱ� ������
			}

			node->x_min = (line->beginVertex.y < line->endVertex.y) ? line->beginVertex.x : line->endVertex.x;
			node->y_max = (line->beginVertex.y < line->endVertex.y) ? line->endVertex.y : line->beginVertex.y;

			edgeTable.insert(pair<LineKey, EdgeNode*>(line->lineKey, node));
		}

		//-----------------------------------------------------------------------
		//ü�� ���̺� ���� �� PreProcessing
		//-----------------------------------------------------------------------
		int totalEdgeSize = listLine.size();
		list<int> upLineSavelist;	//�� ���̶� ���ٷ� �ö� ������ �ε��� ����
		for( int y = 0; y < height; ++y )
		{
			list<Line*> chainLine;

			//for( auto& line : listLine )
			listLine.sort([] (const Line* lhs, const Line* rhs) {	return lhs->lineKey.beginIndex < rhs->lineKey.beginIndex;	 });	//�ε��� ������ �������� ����

			for( auto& lineIter = listLine.begin(); lineIter != listLine.end(); )
			{
				if( y == (*lineIter)->beginVertex.y || y == (*lineIter)->endVertex.y ) //�ش� y�࿡ ������ ��ġ���� �˻�			 	
				{
					chainLine.push_back(*lineIter);
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

					if( (*iter)->lineKey.beginIndex % totalEdgeSize > innerIter->lineKey.beginIndex % totalEdgeSize )	 //�ε����� �׷��� ���� ��
					{
						isOnceCount = false;
						continue;
					}

					if( (*iter)->lineKey.beginIndex == innerIter->lineKey.endIndex ||
						(*iter)->lineKey.endIndex == innerIter->lineKey.beginIndex )	 //���� ����Ǿ������� �Ϲ����� ���
					{
						isOnceCount = false;
						break;
					}
					isOnceCount = true;
				}

				if( isOnceCount )
				{
					if( find(upLineSavelist.begin(), upLineSavelist.end(), (*iter)->lineKey.beginIndex) == upLineSavelist.end() )	//����Ʈ�� ���Ե��� �ʾҴ� ��, �ö��� ����.
					{
						upLineSavelist.push_back((*iter)->lineKey.beginIndex);	//�ѹ� �ö� ���� ����
						((*iter)->beginVertex.y < (*iter)->endVertex.y) ? (++(*iter)->beginVertex.y) : (++(*iter)->endVertex.y);
						listLine.push_back(*iter);	//�ٽ� �˻��ϱ� ���ؼ� ����Ʈ�� �־����
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
		// ������
		//-------------------------------------------------------------------
		list<EdgeNode*> continueRenderLine;
		auto& chainIter = chainTable.begin();
		for( Dint y = (*chainTable.begin()).first;; ++y )	//�������� �׷������� �ϱ� ������ 
		{
			if( y == chainIter->first )	//���� ������ ���̰� ��ġ�� �κ�
			{
				for( auto& chainLine : chainIter->second ) //���� ���̿� ������ ���ε��� ��ȸ
				{
					EdgeNode* node = (*edgeTable.find(chainLine->lineKey)).second;
					continueRenderLine.push_back(node);
				}

				continueRenderLine.sort([] (const EdgeNode* lhs, const EdgeNode* rhs)
				{
					if( lhs->x_min == rhs->x_min )
					{	//���Ⱑ 0�� ��찡 �ֱ� ������ ���� �������δ� ������ �����Ҽ� ��� x_min�� ���� ��������� ���Ѵ�.
						return lhs->x_min + lhs->reverseSlope < rhs->x_min + rhs->reverseSlope;
					}
					return lhs->x_min < rhs->x_min;
				});	//x_min�� ũ�� ������ �������� ����

				if( chainIter != (--chainTable.end()) )	//�˻� ����� ������ �� �������� �̵�
				{
					++chainIter;
				}
			}

			//��� ������ ���----------------		
			Dint beginX = -1;
			Dint endX = -1;
			Dint odd_even = 0;
			for( auto& dotNode : continueRenderLine )
			{
				dotNode->x_min += dotNode->reverseSlope;

				if( odd_even % 2 == 0 ) //¦��
				{
					beginX = (int)ceilf(dotNode->x_min); //������ �ø�				
				}
				else //Ȧ��			
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
				continueRenderLine.end());	//y_max���� ���� y���ΰ� ���ٸ� ����Ʈ���� ����

			//--------------------------------
			if( continueRenderLine.size() == 0 )	//�Ѹ��� �׷����� �������� �׻� �� ����Ʈ�� ���� �����Ѵ�. �������� �������
			{
				Safe_Delete_ListInMap(chainTable);
				Safe_Delete_Map(edgeTable);
				break;
			}
		}
	}
}

