#include "Coold_RenderModule.h"
#include "..\Data\CoolD_Inlines.h"
#include "CoolD_CustomMesh.h"
#include "CoolD_Transform.h"
#include "..\Console\CoolD_Command.h"
#include "..\Thread\CoolD_Thread.h"

namespace CoolD
{
	RenderModule::RenderModule()
		: m_Buffer(nullptr), m_CullMode(BSCullType::CCW), 
		m_pMesh(nullptr), m_pDepthBuffer(nullptr)
	{			
		m_vecLine.resize(100);
		m_edgeTable.resize(100);
		m_activeTable.resize(100);
	}		

	RenderModule::~RenderModule()
	{	
		m_Buffer	= nullptr;		
		m_pMesh		= nullptr;
		m_pDepthBuffer		= nullptr;
	}

	Dvoid RenderModule::ClearColorBuffer(Dulong clearColor)
	{
		Dint totalPixelCount = m_Height * m_Width * 3;
				
		for( Dint i = 0; i < totalPixelCount; i+=3)
		{
			m_Buffer[ i ] = (Duchar)(clearColor >> 24);
			m_Buffer[ i + 1 ] = (Duchar)(clearColor >> 16);
			m_Buffer[ i + 2 ] = (Duchar)(clearColor >> 8);
		}
		
		if (m_pDepthBuffer == nullptr)
		{
			GETSINGLE(DepthBuffer).ClearDepthBuffer(m_Height, m_Width);
			m_pDepthBuffer = &GETSINGLE(DepthBuffer);
			return;
		}

		m_pDepthBuffer->ClearDepthBuffer(m_Height, m_Width);
	}

	Dvoid RenderModule::RenderBegin(CustomMesh* pMesh)
	{		
		if( pMesh == nullptr )
		{
			LOG("Mesh is Null");
			return;
		}		

		m_pMesh = pMesh;		

		if( m_trasnformVertex.capacity() < m_pMesh->GetVertexSize() )
		{
			m_trasnformVertex.resize(m_pMesh->GetVertexSize());
		}		

		AdjustTransform();
		Render();
	}

	Dvoid RenderModule::RenderEnd()
	{
		m_trasnformVertex.clear();
		m_pMesh = nullptr;
	}

	Dvoid RenderModule::AdjustTransform()
	{
		if( m_pMesh == nullptr )
			return;		

		if( m_pMesh->GetType() == MSH )
		{
			m_trasnformVertex = (*m_pMesh->GetVectorVertex());
		}

		else if( m_pMesh->GetType() == PLY )
		{
			for( Duint i = 1; i <= m_pMesh->GetVertexSize(); ++i )
			{
				Vector3 v = TransformHelper::TransformWVP(m_arrayTransform, m_pMesh->GetVertex(i));
				m_trasnformVertex.emplace_back(v);
			}
		}
		else
		{	//Ÿ�������� �� �Ǿ����� ������ ����
			assert(false);
		}
	}

	static VariableCommand cc_use_frustumcull("cc_use_frustumcull", "1");
	static VariableCommand cc_use_backspacecull("cc_use_backspacecull", "1");
	Dvoid RenderModule::Render( )
	{			
		if( m_pMesh == nullptr )
			return;

		vector<BaseFace>* pvecFace = m_pMesh->GetVectorFace();
		for( Dint faceNum = 0; faceNum < (Dint)pvecFace->size(); ++faceNum )
		{			
			BaseFace currentFace = (*pvecFace)[ faceNum ];
						
			//projection frustum culling
			if( cc_use_frustumcull.Bool() && FrustumCulling(currentFace) )
			{
				continue;
			}			

			m_vecLine.clear();
			m_edgeTable.clear();
			m_activeTable.clear();

			CreatePointsToLines(pvecFace, faceNum, m_vecLine);

			//backspace culling
			if( cc_use_backspacecull.Bool() && !BackSpaceCulling(m_vecLine) )
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
				DrawFace(m_activeTable, m_edgeTable, MixDotColor( currentFace.color ));
			}
		}	
		
	}

	//-----------------------------------------------------------------------
	//�� -> �� �����
	//-----------------------------------------------------------------------
	Dvoid RenderModule::CreatePointsToLines( const vector<BaseFace>* pvecFace, Duint faceNum, vector<Line>& vecLine)
	{
		const BaseFace& face = (*pvecFace)[ faceNum ];

		Duint faceSize = face.vecIndex.size();

		InsertPointToLine(vecLine, face, faceSize - 1, 0);

		for( Duint j = 0; j < faceSize - 1; ++j )
		{
			InsertPointToLine(vecLine, face, j, j + 1);
		}
	}
	//-----------------------------------------------------------------------
	//���� ���̺� �����
	//-----------------------------------------------------------------------
	Dvoid RenderModule::CreateEdgeTable(vector<Line>& vecLine, vector<LineEdge>& edgeTable)
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
	Dvoid RenderModule::CreateChainTable(vector<Line>& vecLine, vector<ActiveLine>& activeTable)
	{
		assert(!vecLine.empty());
		
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
				auto checkOneCountIter = STD_FIND_IF(currentLine, [&] (const Line& line){	return CheckContinueLine(chainIter->lineKey, line.lineKey);		});
				if( checkOneCountIter == currentLine.end() )	//Ư�� ������ ����Ǿ� ���� �ʴ� ( 1���� ī��Ʈ �� 14_ScanLineFill.pdf ���� )
				{					
					if( chainIter->isOneCount == false )
					{
						chainIter->isOneCount = true;
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

	Dbool RenderModule::BackSpaceCulling( vector<Line>& vecLine )
	{
		if( vecLine.size() > 2 )
		{	
			Vector3 v1(0, 0, -1);
			Vector3 v2 = (vecLine[ 0 ].endVertex - vecLine[ 0 ].beginVertex).Cross(vecLine[ 1 ].endVertex - vecLine[ 1 ].beginVertex);			
			v2.Normalize();
			
			switch( m_CullMode )
			{
			case BSCullType::CW:
				if( v1.Dot(v2) <= 0 )
				{
					return true;
				}
				break;
			case BSCullType::CCW:
				if( v1.Dot(v2) >= 0 )
				{
					return true;
				}
				break;				
			default:	//BSCullType::ALL
				return true;	
			}			
		}

		return false;
	}

	static VariableCommand cc_frustum_bias("cc_frustum_bias", "0.1f");
	Dbool RenderModule::FrustumCulling(const BaseFace& currentface)
	{			
		Duint faceSize = currentface.vecIndex.size();
		Dbool isCulling = false;

		Dfloat boundary = 1.0f + cc_frustum_bias.Float();

		for( Duint i = 0; i < faceSize; ++i )
		{
			Dint index = currentface.vecIndex[ i ];
			Vector3 Vertex = m_trasnformVertex[ index - 1 ];

			if( Vertex.x < -boundary || boundary < Vertex.x ||
				Vertex.y < -boundary || boundary < Vertex.y ||
				Vertex.z < -boundary || boundary < Vertex.z )
			{	//�þ� ������ �Ѿ ��, ���� ��
				isCulling = true;
			}
			else
			{
				isCulling = false;
			}
		}

		return isCulling;
	}
	//-------------------------------------------------------------------
	//�ش� ��ġ �� ���
	//-------------------------------------------------------------------
	Dvoid RenderModule::DrawDot(const Duint& x, const Duint& y, const Dulong& DotColor)
	{	
		Dint index = (((m_Width * y) + x) * 3);

		m_Buffer[ index ] = (Duchar)(DotColor >> 24);
		m_Buffer[ index + 1 ] = (Duchar)(DotColor >> 16);
		m_Buffer[ index + 2 ] = (Duchar)(DotColor >> 8);
	}

	//-------------------------------------------------------------------
	//���� ������ ��� ���� �� ���
	//-------------------------------------------------------------------
	Dvoid RenderModule::DrawLine(vector<EdgeNode>& renderLine, const Dint currentHeight, const Dulong dotColor)
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

				if (m_pDepthBuffer && m_pDepthBuffer->DepthTest(i, currentHeight, resultDepth))
				{
					DrawDot(i, currentHeight, dotColor);
				}
			}			
		}		
	}	
	
	//-------------------------------------------------------------------
	// �޽��� �ش� �� �׸���
	//-------------------------------------------------------------------
	Dvoid RenderModule::DrawFace(vector<ActiveLine>& activeTable, vector<LineEdge>& edgeTable, const Dulong dotColor)
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

	void RenderModule::SetScreenInfo(Dvoid* buffer, const int width, const int height)
	{
		if( m_Buffer != buffer )
		{
			m_Buffer = (Duchar*)buffer;
			m_Width = width;
			m_Height = height;
		}		
	}	

	Dvoid RenderModule::InsertPointToLine(vector<Line>& vecLine, const BaseFace& face, Duint firstIndex, Duint secondIndex )
	{			
		LineKey lineKey;
		lineKey.beginIndex = face.vecIndex[ firstIndex ];
		lineKey.endIndex = face.vecIndex[ secondIndex ];

		Vector3 beginVertex = TransformHelper::TransformViewport(m_arrayTransform, m_trasnformVertex[ lineKey.beginIndex - 1 ]);
		Vector3 endVertex = TransformHelper::TransformViewport(m_arrayTransform, m_trasnformVertex[ lineKey.endIndex - 1 ]);

		beginVertex.x = floor(beginVertex.x);
		beginVertex.y = floor(beginVertex.y);
		endVertex.x = floor(endVertex.x);
		endVertex.y = floor(endVertex.y);

		vecLine.emplace_back(lineKey, beginVertex, endVertex);
	}		
	
	//------------------------------------------------------------------------------------------------

	DepthBuffer::DepthBuffer()
		:m_width(-1), m_DepthBuffer(nullptr)
	{

	}

	DepthBuffer::~DepthBuffer()
	{
		Safe_Delete_Array(m_DepthBuffer);
	}

	void DepthBuffer::ClearDepthBuffer(Dint height, Dint width)
	{
		m_width = width;
		Dint totalDepthClearCount = height * width;

		if (m_DepthBuffer == nullptr)
		{
			m_DepthBuffer = new Dfloat[totalDepthClearCount];
		}

		memset(m_DepthBuffer, 0, totalDepthClearCount * sizeof(Dfloat));		//0���� �ʱ�ȭ �ϰ� ���� �׽�Ʈ�Ҷ��� 1 - ���� ������ ���Ѵ�.
	}

	Dbool DepthBuffer::DepthTest( Dint x, Dint y, Dfloat depth)
	{
		//
		Dfloat opDepth = 1.0f - depth;

		Dfloat* buffer = &m_DepthBuffer[y * m_width + x];
		if (*buffer < opDepth)
		{
			*buffer = opDepth;
			return true;
		}

		return false;
	}
}