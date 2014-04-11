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
		//chrono::milliseconds mill = chrono::duration_cast<chrono::milliseconds>(end - start);		//test �ð� ����						
	}

	//-----------------------------------------------------------------------
	//�� -> �� �����
	//-----------------------------------------------------------------------
	tuple_OptimizeY AreaFilling::CreatePointsToLines(tuple_meshInfo& meshInfo, Duint faceNum)
	{		
		//���� ���� �ȿ��� ����ϱ� ���ؼ� ����� ���
		atomic<Dfloat> minOptimizeY = 99999.0f;
		atomic<Dfloat> maxOptimizeY = -1.0f;
		vector<Line> vecLine; //����ȭ ������ ����ø��� ���ؼ� �߰�

		const BaseFace& face = get<1>(meshInfo)[faceNum];

		//parallel_for������ �Ϲ� ���� �� �����ϴ� �� unsigned ��� ���Ѵ�.---------------------
		//parallel_for(0, (Dint)face.vecIndex.size(), [&](Dint j)
		for( Duint j = 0; j < face.vecIndex.size(); ++j )
		{
			LineKey lineKey;
			lineKey.beginIndex = face.vecIndex[ j ];
					
			//���� ����
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
						
			//x, y ���и� �ø� ( z�� 0~1 ������ ���� �� )
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

		m_VecLine.swap(vecLine);	//���� �����ȿ����� m_VecLine�� ������ ���������� �ʱ� ������ �ܺο��� ����
		return make_tuple(minOptimizeY.load(), maxOptimizeY.load());
		//--------------------------------------------------
	}

	//-----------------------------------------------------------------------
	//���� ���̺� �����
	//-----------------------------------------------------------------------
	Dvoid AreaFilling::CreateEdgeTable( )
	{
		assert( !m_VecLine.empty());		

		//for(const auto& line : m_VecLine )
		parallel_for_each(begin(m_VecLine), end(m_VecLine), [this](const Line& line)
		{ //�� ���� �������� m_VecLine�� ������ ���������������� �б⸸ �ϹǷ� �����ȵ�
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

			m_EdgeTable.push_back(LineEdge(line.lineKey, node));
		});
	}

	//-----------------------------------------------------------------------
	//ü�� ���̺� ���� �� PreProcessing
	//-----------------------------------------------------------------------
	Dvoid AreaFilling::CreateChainTable(tuple_OptimizeY opY)
	{
		assert(!m_VecLine.empty());
		
		int totalEdgeSize = m_VecLine.size();
		list<int> upLineSavelist;	//�� ���̶� ���ٷ� �ö� ������ �ε��� ����

		//parallel_buffered_sort�� ����Ʈ���� �����Ҽ� ����. ���ӵ� �޸𸮰� �ƴϱ� ������ end�� begin�� �޸� ���̸� ���Ҽ� ����.
		//m_VecLine.sort([] (const Line& lhs, const Line& rhs) {	return lhs.lineKey.beginIndex < rhs.lineKey.beginIndex;	 });	//�ε��� ������ �������� ����
		parallel_buffered_sort(begin(m_VecLine), end(m_VecLine), [](const Line& lhs, const Line& rhs) {	return lhs.lineKey.beginIndex < rhs.lineKey.beginIndex;	 });

		//�������� m_VecLine�� ��ȭ�ϸ鼭 ���� ���ο� ������ ��ġ�Ƿ� ����ȭ X
		Dint minY = (Dint)(get<0>(opY));
		Dint maxY = (Dint)(get<1>(opY));

		for (Dint y = minY; y < maxY; ++y)
		{
			list<Line> currentLine;			

			for( auto& lineIter = m_VecLine.begin(); lineIter != m_VecLine.end(); )
			{			
				ITER_CONVERT(pLine, &(*lineIter));
				if (y == pLine->beginVertex.y || y == pLine->endVertex.y) //�ش� y�࿡ ������ ��ġ���� �˻�			 	
				{
					currentLine.emplace_back( *pLine );
					lineIter = m_VecLine.erase( lineIter );
				}
				else
				{
					++lineIter;
				}
			} // ���� ���ο� ���԰����� ������ ��� ����

			for( auto& chainIter = currentLine.begin(); chainIter != currentLine.end(); )
			{
				Dbool isOnceCount = true;
				for( auto& innerIter : currentLine )
				{
					if( chainIter->lineKey == innerIter.lineKey )	//������ ����
					{
						continue;
					}

					if( chainIter->lineKey.beginIndex % totalEdgeSize > innerIter.lineKey.beginIndex % totalEdgeSize )	 //�ε����� �׷��� ���� ��
					{
						isOnceCount = false;
						continue;
					}

					if( CheckContinueLine( chainIter->lineKey, innerIter.lineKey) )	 //���� ����Ǿ������� �Ϲ����� ���
					{
						isOnceCount = false;
						break;
					}
					isOnceCount = true;
				}

				if( isOnceCount )
				{
					if( find(upLineSavelist.begin(), upLineSavelist.end(), chainIter->lineKey.beginIndex) == upLineSavelist.end() )	//����Ʈ�� ���Ե��� �ʾҴ� ��, �ö��� ����.
					{						
						upLineSavelist.push_back( chainIter->lineKey.beginIndex );	//�ѹ� �ö� ���� ����
						++(chainIter->GetMinY());			//y�� �� 1�ø���								
						m_VecLine.emplace_back( *chainIter );//�ٽ� �˻��ϱ� ���ؼ� ����Ʈ�� �־����						
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
	//�ش� ��ġ �� ���
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
	//���� ������ ��� ���� �� ���
	//-------------------------------------------------------------------
	Dvoid AreaFilling::DrawLine(vector<EdgeNode>& renderLine, const Dint currentHeight, const Dulong dotColor)
	{
		Dint beginX = -1;
		Dint endX = -1;
		Dint odd_even = 0;
		Dfloat DepthLeft, DepthRight;

		//������� �����ؾ��� ���� X
		for( auto& dotNode : renderLine )
		{			
			if( odd_even % 2 == 0 ) //¦��
			{
				beginX = (int)ceilf(dotNode.x_min); //������ �ø�				
				
				//���� ��ġ������ ���̰� ����
				Dfloat dy = dotNode.y_max - dotNode.y_min;
				Dfloat dz = dotNode.max_depth - dotNode.min_depth;
				Dfloat rate = (currentHeight - dotNode.y_min )/ dy;
				DepthLeft = dotNode.min_depth + ( dz * rate );
			}
			else //Ȧ��
			{
				endX = (int)ceilf(dotNode.x_min);
				//���� ��ġ������ ���̰� ����
				Dfloat dy = dotNode.y_max - dotNode.y_min;
				Dfloat dz = dotNode.max_depth - dotNode.min_depth;
				Dfloat rate = (currentHeight - dotNode.y_min) / dy;
				DepthRight = dotNode.min_depth + (dz * rate);


				//for( Dint i = beginX; i < endX; ++i )
				parallel_for(beginX, endX, 1, [&]( Dint i )
				{
					//��, �� ������ ���̰� ����
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
	// �޽��� �ش� �� �׸���
	//-------------------------------------------------------------------
	Dvoid AreaFilling::DrawFace(const Dulong dotColor)
	{		
		assert(m_Buffer != nullptr && !m_ActiveTable.empty());

		vector<EdgeNode> continueRenderLine;	//�� �����̳ʴ� erase�� ����ؾ��ϱ� ������ ���������̳ʷ� �ϸ� �ȵȴ�.
		auto& activeIter = m_ActiveTable.begin();

		//m_ActiveTable.sort([] (const ActiveLine& lhs, const ActiveLine& rhs) -> Dbool { return lhs.height < rhs.height; } );
		parallel_buffered_sort(begin(m_ActiveTable), end(m_ActiveTable), [](const ActiveLine& lhs, const ActiveLine& rhs) -> Dbool { return lhs.height < rhs.height; });
		for (Dint y = activeIter->height;; ++y)	//�������� �׷������� �ϱ� ������ ActiveTable���� ���۰��� ���̷� �ʱ�ȭ
		{
			if( y == activeIter->height )	//���� ������ ���̰� ��ġ�� �κ�
			{
				list<Line>& currentLine = activeIter->currentLine;
				//parallel_for_each(begin(currentLine), end(currentLine), [&](const Line& line)	//continueRenderLine�� ������ ������ ���� �ʱ� ������ ���X
				for( auto& line : activeIter->currentLine ) //���� ���̿� ������ ���ε��� ��ȸ				
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
				Clear();
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
	//y�� ������ �Ϸķ� ������� üũ (��, �ﰢ���� �̷�� ���� ���� ��)
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
		
		return true; //y���� ��ġ�Ѵ� �� �׷����� �ȵȴ�.
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