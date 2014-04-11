#pragma once

#include "..\Data\CoolD_Defines.h"
#include "..\Data\CoolD_Type.h"
#include "..\Data\CoolD_Struct.h"
#include "..\Math\CoolD_Matrix44.h"
#include <concurrent_vector.h>

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
							
		//병렬 루프에서는 스레드 세이프 컨테이너를 사용해야 함 다만 병렬 컨터이너는 emplace_back을 사용할수 없다. 
		//또한 erase멤버 함수가 없기 때문에...이는 추후에 알아보고 적용하자
		vector<Line> m_VecLine;	
		vector<ActiveLine> m_ActiveTable;
		concurrent_vector<LineEdge> m_EdgeTable;	 	

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
		tuple_OptimizeY CreatePointsToLines(tuple_meshInfo& meshInfo, Duint faceNum);
		Dvoid CreateEdgeTable();
		Dvoid CreateChainTable(tuple_OptimizeY opY);

	private:
		Dbool DepthTest(const Duint x, const Duint y, Dfloat depth);

	private:	//그려지는 단계
		Dvoid DrawDot(const Duint x, const Duint y, const Dulong DotColor);
		Dvoid DrawLine(vector<EdgeNode>& renderLine, const Dint currentHeight, const Dulong dotColor);
		Dvoid DrawFace(const Dulong dotColor);

	private:	//그 외 보조 함수
		Dulong MixDotColor( const BaseColor& color );
		Dbool CheckContinueLine( const LineKey& lhs, const LineKey& rhs ) const;	
		Dbool CheckYStandInLine(  ) const;
		Dfloat GetBigYValue(const Vector3& begin, const Vector3& end) const ;
		Dfloat GetSmallYValue(const Vector3& begin, const Vector3& end) const;
	
	private:
		Dvoid Clear();
	};
}