#pragma once

#include "..\Data\CoolD_Defines.h"
#include "..\Data\CoolD_Type.h"
#include "..\Data\CoolD_Struct.h"
#include "..\Math\CoolD_Matrix44.h"

namespace CoolD
{
	class CustomMesh;
	class AreaFilling final	//상속 받을일은 없을 듯
	{
	private:
		Duchar* m_Buffer;	//프레임 버퍼
		Dfloat* m_DepthBuffer;	//깊이 버퍼
		Dint m_Width;
		Dint m_Height;			
			
		list<Line> m_ListLine;		
		list<ActiveLine> m_ActiveTable;
		list<LineEdge> m_EdgeTable;		

		array<Matrix44, TRANSFORM_END> m_arrayTransform;		

	public:
		AreaFilling();
		~AreaFilling();
		
	public:
		Dvoid Render(tuple_meshInfo& meshInfo);
		void SetTransform(TransType type, const Matrix44 matrix);
		void SetScreenInfo(Dvoid* buffer, const int width, const int height);
		const array<Matrix44, TRANSFORM_END>& GetArrayTransform();

	private:	//렌더 관련 데이터 생성부			
		Dvoid CreatePointsToLines(tuple_meshInfo meshInfo, Duint faceNum);
		Dvoid CreateEdgeTable();
		Dvoid CreateChainTable( );	

	private:
		Dbool DepthTest(const Duint x, const Duint y, Dfloat depth);
	private:	//그려지는 단계
		Dvoid DrawDot(const Duint x, const Duint y, const Dulong DotColor);
		Dvoid DrawLine(list<EdgeNode>& renderLine, const Dint currentHeight, const Dulong dotColor);
		Dvoid DrawFace(const Dulong dotColor);

	private:	//그 외 보조 함수
		Dulong MixDotColor( const BaseColor& color );
		Dbool CheckContinueLine( const LineKey& lhs, const LineKey& rhs ) const;		
	};
}