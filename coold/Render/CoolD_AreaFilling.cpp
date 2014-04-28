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
		
		memset(m_DepthBuffer, 0, totalDepthClearCount * sizeof(Dfloat));		//0���� �ʱ�ȭ �ϰ� ���� �׽�Ʈ�Ҷ��� 1 - ���� ������ ���Ѵ�.
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
			{	//��� ����� y ���� ������ üũ
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
		//	{	//��� ����� y ���� ������ üũ
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
	//�� -> �� �����
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
	//���� ���̺� �����
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
			{	//�и� 0�̸� ������ 0 ZeroDivide����
				node.reverseSlope = 0;
			}
			else
			{
				node.reverseSlope = dx / dy;
			}			

			if( line.beginVertex.y < line.endVertex.y )
			{
				node.x_min		= line.beginVertex.x;
				node.y_min		= line.beginVertex.y;	//���� ������ ���ؼ� �߰�
				node.y_max		= line.endVertex.y;
				node.min_depth	= line.beginVertex.z;	//���� ������ ���ؼ� �߰�
				node.max_depth  = line.endVertex.z;		//���� ������ ���ؼ� �߰�
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
	//ü�� ���̺� ���� �� PreProcessing
	//-----------------------------------------------------------------------
	Dvoid AreaFilling::CreateChainTable(vector<Line>& vecLine, vector<ActiveLine>& activeTable)
	{
		assert(!vecLine.empty());
		vector<Dint> vecUpLineSave;	//�� ���̶� ���ٷ� �ö� ������ �ε��� ����
		
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
				if (y == pLine->beginVertex.y || y == pLine->endVertex.y) //�ش� y�࿡ ������ ��ġ���� �˻�			 	
				{
					currentLine.emplace_back( *pLine );
					lineIter = vecLine.erase(lineIter);
				}
				else
				{
					++lineIter;
				}
			} // ���� ���ο� ���԰����� ������ ��� ����
						
			if( currentLine.empty() )
			{
				continue;
			}		
			
			for( auto chainIter = currentLine.begin(); chainIter != currentLine.end(); )
			{
				auto retIter = STD_FIND_IF(currentLine, [&] (const Line& line){	return CheckContinueLine(chainIter->lineKey, line.lineKey);		});
				if( retIter == currentLine.end() )	//Ư�� ������ ����Ǿ� ���� �ʴ� ( 1���� ī��Ʈ �� 14_ScanLineFill.pdf ���� )
				{
					if( find(vecUpLineSave.begin(), vecUpLineSave.end(), chainIter->lineKey.beginIndex) == vecUpLineSave.end() )	//����Ʈ�� ���Ե��� �ʾҴ� ��, �ö��� ����.
					{
						vecUpLineSave.push_back(chainIter->lineKey.beginIndex);	//�ѹ� �ö� ���� ����
						++(chainIter->GetMinY());		 //y�� �� 1�ø���								
						vecLine.emplace_back(*chainIter);//�ٽ� �˻��ϱ� ���ؼ� ����Ʈ�� �־����						
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
	//�ش� ��ġ �� ���
	//-------------------------------------------------------------------
	Dvoid AreaFilling::DrawDot(const Duint& x, const Duint& y, const Dulong& DotColor)
	{	
		Dint index = (((m_Width * y) + x) * 3);

		m_Buffer[ index ] = (Duchar)(DotColor >> 24);
		m_Buffer[ index + 1 ] = (Duchar)(DotColor >> 16);
		m_Buffer[ index + 2 ] = (Duchar)(DotColor >> 8);
	}

	//-------------------------------------------------------------------
	//���� ������ ��� ���� �� ���
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
				//��, �� ������ ���̰� ����					
				Dfloat resultDepth = GetDepthInterpolation(endX, beginX, DepthRight, DepthLeft, i);

				if( DepthTest(i, currentHeight, resultDepth) )
				{
					DrawDot(i, currentHeight, dotColor);
				}
			}			
		}		
	}	
	
	//-------------------------------------------------------------------
	// �޽��� �ش� �� �׸���
	//-------------------------------------------------------------------
	Dvoid AreaFilling::DrawFace(vector<ActiveLine>& activeTable, vector<LineEdge>& edgeTable, const Dulong dotColor)
	{		
		assert(m_Buffer != nullptr && !activeTable.empty());

		vector<EdgeNode> continueRenderLine;
		auto activeIter = activeTable.begin();
		
		for (Dint y = activeIter->height;; ++y)	//�������� �׷������� �ϱ� ������ ActiveTable���� ���۰��� ���̷� �ʱ�ȭ
		{
			if( y == activeIter->height )	//���� ������ ���̰� ��ġ�� �κ�
			{
				for( auto& line : activeIter->currentLine ) //���� ���̿� ������ ���ε��� ��ȸ				
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
					{	//���Ⱑ 0�� ��찡 �ֱ� ������ ���� �������δ� ������ �����Ҽ� ��� x_min�� ���� ��������� ���Ѵ�.
						return lhs.x_min + lhs.reverseSlope < rhs.x_min + rhs.reverseSlope;
					}
					return lhs.x_min < rhs.x_min;
				});	//x_min�� ũ�� ������ �������� ����
				
				if (activeIter != (--activeTable.end()))	//�˻� ����� ������ �� �������� �̵�
				{
					++activeIter;
				}
			}

			DrawLine(continueRenderLine, y, dotColor);	//���� ���� ��Ʈ ���

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