#pragma once

#include "CoolD_Defines.h"
#include "CoolD_Type.h"

namespace CoolD
{
	class CustomMesh;
	class AreaFilling final	//��� �������� ���� ��
	{
	private:
		Buffer* const m_Buffer;	//Null�ϼ��� ����
		Dint m_Width;
		Dint m_Height;
		list<CustomMesh*> m_ListMesh;	//�޽� ����Ʈ�� ����
		list<Line> m_ListLine;
		
		list<ActiveLine> m_ActiveTable;
		list<LineEdge> m_EdgeTable;

	public:
		AreaFilling( void* buffer, const int width, const int height );
		~AreaFilling();
		
	public:		
		void AddMesh( CustomMesh* const pMesh );
		void Render();

	private:
		const BaseFace& CreatePointsToLines(const CustomMesh* pMesh, Duint faceNum);
		void CreateEdgeTable();
		void CreateChainTable( );	

	private:
		void DrawDot(const Duint x, const Duint y, const Dulong DotColor);
		void DrawLine(list<EdgeNode>& renderLine, const Dint currentHeight, const Dulong dotColor);
		void DrawFace(const Dulong dotColor);

	private:		
		Dulong MixDotColor( const BaseColor& color );
		bool CheckContinueLine( const LineKey& lhs, const LineKey& rhs ) const;		
	};
}