#pragma once

#include "CoolD_Defines.h"
#include "CoolD_Type.h"
#include "CoolD_Struct.h"

namespace CoolD
{
	class CustomMesh;
	class AreaFilling final	//상속 받을일은 없을 듯
	{
	private:
		Duchar* const m_Buffer;	//Null일수가 없음
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

	private:	//렌더 관련 데이터 생성부	
		const BaseFace& CreatePointsToLines(const CustomMesh* pMesh, Duint faceNum);
		Dvoid CreateEdgeTable();
		Dvoid CreateChainTable( );	

	private:	//그려지는 단계
		Dvoid DrawDot(const Duint x, const Duint y, const Dulong DotColor);
		Dvoid DrawLine(list<EdgeNode>& renderLine, const Dint currentHeight, const Dulong dotColor);
		Dvoid DrawFace(const Dulong dotColor);

	private:	//그 외 보조 함수
		Dulong MixDotColor( const BaseColor& color );
		Dbool CheckContinueLine( const LineKey& lhs, const LineKey& rhs ) const;		
	};
}