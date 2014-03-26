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
		assert(!m_ListMesh.empty());	//��� ������ �翬�� �ȵ�

		for( auto& mesh : m_ListMesh )
		{			
			TimeForm start = chrono::system_clock::now();


			for( Duint faceNum = 1; faceNum <= mesh->GetFaceSize(); ++faceNum )
			{
				const BaseFace& face = CreatePointsToLines(mesh, faceNum);	//�޽����� � ���� �׸� �� ����
				CreateEdgeTable( );	
				CreateChainTable( );
				DrawFace( MixDotColor(face.color) );
			}

			TimeForm end = chrono::system_clock::now();
			chrono::milliseconds mill = chrono::duration_cast<chrono::milliseconds>(end - start);		//test �ð� ����				
		}
	}

	//-----------------------------------------------------------------------
	//�� -> �� �����
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

			m_ListLine.emplace_back( lineKey, beginVertex, endVertex );	//�ӽ� ��ü �����ϸ鼭 ����
		}

		return face;
	}

	//-----------------------------------------------------------------------
	//���� ���̺� �����
	//-----------------------------------------------------------------------
	void AreaFilling::CreateEdgeTable( )
	{
		assert( !m_ListLine.empty() );	//��� ������ �翬�� �ȵ�

		for(const auto& line : m_ListLine )
		{
			EdgeNode node;
			Dfloat dx = line.endVertex.x - line.beginVertex.x;
			Dfloat dy = line.endVertex.y - line.beginVertex.y;

			if( dx == 0 || dy == 0 )
			{	//�и� 0�̸� ������ 0 ZeroDivide����
				node.reverseSlope = 0;
			}
			else
			{
				node.reverseSlope = dx / dy; // dx/dy�� �����̰� �� �������� �����ؾ��ϱ� ������
			}

			node.x_min = (line.beginVertex.y < line.endVertex.y) ? line.beginVertex.x : line.endVertex.x;
			node.y_max = (line.beginVertex.y < line.endVertex.y) ? line.endVertex.y : line.beginVertex.y;

			m_EdgeTable.emplace_back( line.lineKey, node );
		}
	}

	//-----------------------------------------------------------------------
	//ü�� ���̺� ���� �� PreProcessing
	//-----------------------------------------------------------------------
	void AreaFilling::CreateChainTable( )
	{
		assert( !m_ListLine.empty() );	//��� ������ �翬�� �ȵ�

		int totalEdgeSize = m_ListLine.size();
		list<int> upLineSavelist;	//�� ���̶� ���ٷ� �ö� ������ �ε��� ����

		m_ListLine.sort([] (const Line& lhs, const Line& rhs) {	return lhs.lineKey.beginIndex < rhs.lineKey.beginIndex;	 });	//�ε��� ������ �������� ����

		for( int y = 0; y < m_Height; ++y )
		{
			list<Line> currentLine;			

			for( auto& lineIter = m_ListLine.begin(); lineIter != m_ListLine.end(); )
			{			
				if( y == (*lineIter).beginVertex.y || y == (*lineIter).endVertex.y ) //�ش� y�࿡ ������ ��ġ���� �˻�			 	
				{
					currentLine.emplace_back( *lineIter );
					lineIter = m_ListLine.erase(lineIter);
				}
				else
				{
					++lineIter;
				}
			} // ���� ���ο� ���԰����� ������ ��� ����

			for( auto& chainIter = currentLine.begin(); chainIter != currentLine.end(); )
			{
				bool isOnceCount = true;
				for( auto& innerIter : currentLine )
				{
					if( (*chainIter).lineKey == innerIter.lineKey )	//������ ����
					{
						continue;
					}

					if( (*chainIter).lineKey.beginIndex % totalEdgeSize > innerIter.lineKey.beginIndex % totalEdgeSize )	 //�ε����� �׷��� ���� ��
					{
						isOnceCount = false;
						continue;
					}

					if( CheckContinueLine( (*chainIter).lineKey, innerIter.lineKey) )	 //���� ����Ǿ������� �Ϲ����� ���
					{
						isOnceCount = false;
						break;
					}
					isOnceCount = true;
				}

				if( isOnceCount )
				{
					if( find(upLineSavelist.begin(), upLineSavelist.end(), (*chainIter).lineKey.beginIndex) == upLineSavelist.end() )	//����Ʈ�� ���Ե��� �ʾҴ� ��, �ö��� ����.
					{						
						upLineSavelist.push_back( (*chainIter).lineKey.beginIndex );	//�ѹ� �ö� ���� ����
						++(*chainIter).GetMinY();			//y�� �� 1�ø���								
						m_ListLine.emplace_back( *chainIter );//�ٽ� �˻��ϱ� ���ؼ� ����Ʈ�� �־����						
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
	//�ش� ��ġ �� �ϳ� ���
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
	//���� ������ ��� ���� �� ���
	//-------------------------------------------------------------------
	void AreaFilling::DrawLine( list<EdgeNode>& renderLine, const Dint currentHeight, const Dulong dotColor)
	{
		Dint beginX = -1;
		Dint endX = -1;
		Dint odd_even = 0;
		for( auto& dotNode : renderLine )
		{
			dotNode.x_min += dotNode.reverseSlope;

			if( odd_even % 2 == 0 ) //¦��
			{
				beginX = (int)ceilf(dotNode.x_min); //������ �ø�				
			}
			else //Ȧ��			
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
	// �޽��� �ش� �� �׸���
	//-------------------------------------------------------------------
	void AreaFilling::DrawFace(const Dulong dotColor)
	{
		assert( !m_ActiveTable.empty() );

		list<EdgeNode> continueRenderLine;
		auto& activeIter = m_ActiveTable.begin();

		m_ActiveTable.sort([] (const ActiveLine& lhs, const ActiveLine& rhs) -> bool { return lhs.height < rhs.height; } );
		for( Dint y = (*m_ActiveTable.begin()).height;; ++y )	//�������� �׷������� �ϱ� ������ 
		{
			if( y == (*activeIter).height )	//���� ������ ���̰� ��ġ�� �κ�
			{
				for( auto& line : (*activeIter).currentLine ) //���� ���̿� ������ ���ε��� ��ȸ
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
					{	//���Ⱑ 0�� ��찡 �ֱ� ������ ���� �������δ� ������ �����Ҽ� ��� x_min�� ���� ��������� ���Ѵ�.
						return lhs.x_min + lhs.reverseSlope < rhs.x_min + rhs.reverseSlope;
					}
					return lhs.x_min < rhs.x_min;
				});	//x_min�� ũ�� ������ �������� ����

				if( activeIter != (--m_ActiveTable.end()) )	//�˻� ����� ������ �� �������� �̵�
				{
					++activeIter;
				}
			}

			DrawLine(continueRenderLine, y, dotColor);	//���� ���� ��Ʈ ���

			//continueRenderLine.erase( remove_if(continueRenderLine.begin(), continueRenderLine.end(),
			//	[y] (const EdgeNode& node)	{	return node.y_max == y; }), continueRenderLine.end() );	//y_max���� ���� y���ΰ� ���ٸ� ����Ʈ���� ����			
			STD_ERASE(continueRenderLine, STD_REMOVE_IF(continueRenderLine, [y] (const EdgeNode& node){return node.y_max == y; }));
			//--------------------------------
			if( continueRenderLine.size() == 0 )	//�� ���� �׷����� �������� �׻� �� ����Ʈ�� ���� �����Ѵ�. �������� �������
			{
				m_ActiveTable.clear();
				m_EdgeTable.clear();
				m_ListLine.clear();
				break;
			}
		}
	}

	//-------------------------------------------------------------------
	//��RGBA�� �����Ͽ� �÷� ������
	//-------------------------------------------------------------------
	Dulong AreaFilling::MixDotColor(const BaseColor& color)
	{
		return (color.r << 24) + (color.g << 16) + (color.b << 8) + (color.a);
	}

	//-------------------------------------------------------------------
	//���� ���� ���� Ȯ��
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