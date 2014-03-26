#pragma once

#include "CoolD_Defines.h"
#include "CoolD_Type.h"

namespace CoolD
{
	class CustomMesh;
	class AreaFilling final	//상속 받을일은 없을 듯
	{
	private:
		Buffer* const m_Buffer;	//Null일수가 없음
		Dint m_Width;
		Dint m_Height;
		list<CustomMesh*> m_ListMesh;	//메쉬 리스트로 관리
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