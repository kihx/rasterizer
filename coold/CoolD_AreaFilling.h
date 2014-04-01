#pragma once

#include "CoolD_Defines.h"
#include "CoolD_Type.h"
#include "CoolD_Struct.h"

namespace CoolD
{
	class CustomMesh;
	class AreaFilling final	//��� �������� ���� ��
	{
	private:
		Duchar* const m_Buffer;	//Null�ϼ��� ����
		Dint m_Width;
		Dint m_Height;				
		list<Line> m_ListLine;
		
		list<ActiveLine> m_ActiveTable;
		list<LineEdge> m_EdgeTable;

	public:
		AreaFilling( Dvoid* buffer, const int width, const int height );
		~AreaFilling();
		
	public:				
		Dvoid Render( const CustomMesh* pMesh );

	private:	//���� ���� ������ ������	
		const BaseFace& CreatePointsToLines(const CustomMesh* pMesh, Duint faceNum);
		Dvoid CreateEdgeTable();
		Dvoid CreateChainTable( );	

	private:	//�׷����� �ܰ�
		Dvoid DrawDot(const Duint x, const Duint y, const Dulong DotColor);
		Dvoid DrawLine(list<EdgeNode>& renderLine, const Dint currentHeight, const Dulong dotColor);
		Dvoid DrawFace(const Dulong dotColor);

	private:	//�� �� ���� �Լ�
		Dulong MixDotColor( const BaseColor& color );
		Dbool CheckContinueLine( const LineKey& lhs, const LineKey& rhs ) const;		
	};
}