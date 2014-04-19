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
							
		array<Matrix44, TRANSFORM_END> m_arrayTransform;	

	public:
		AreaFilling();
		~AreaFilling();
				
	public:
		Dvoid ClearColorBuffer(Dulong clearColor);
		Dvoid Render(const vector<Vector3>* pvecVertex,const vector<BaseFace>* pvecFace);
		void SetScreenInfo(Dvoid* buffer, const int width, const int height);

		inline void SetTransform(const TransType& type, const Matrix44& matrix)	{ m_arrayTransform[ type ] = matrix; }
		inline const array<Matrix44, TRANSFORM_END>& GetArrayTransform()		{ return m_arrayTransform; }

	private:	//렌더 관련 데이터 생성부			
		Dvoid CreatePointsToLines(const vector<Vector3>* pvecVertex, const vector<BaseFace>* pvecFace, Duint faceNum, vector<Line>& vecLine);
		Dvoid CreateEdgeTable(vector<Line>& vecLine, vector<LineEdge>& edgeTable);
		Dvoid CreateChainTable(vector<Line>& vecLine, vector<ActiveLine>& activeTable);

	private:
		Dbool DepthTest(const Duint x, const Duint y, Dfloat depth);

	private:	//그려지는 단계
		Dvoid DrawDot(const Duint& x, const Duint& y, const Dulong& DotColor);
		Dvoid DrawLine(vector<EdgeNode>& renderLine, const Dint currentHeight, const Dulong dotColor);
		Dvoid DrawFace(vector<ActiveLine>& activeTable, vector<LineEdge>& edgeTable, const Dulong dotColor);
		
	private:	//그 외 보조 함수				
		Dvoid InsertPointToLine(vector<Line>& vecLine, LineKey& lineKey, Vector3& beginVertex, Vector3& endVertex);

		inline Dulong MixDotColor(const BaseColor& color){ return (color.r << 24) + (color.g << 16) + (color.b << 8) + (color.a); }
		inline Dbool CheckContinueLine(const LineKey& lhs, const LineKey& rhs) const { return (lhs.beginIndex == rhs.endIndex || lhs.endIndex == rhs.beginIndex); }
		inline Dfloat GetDepthInterpolation(const Dfloat& max, const Dfloat& min, const Dfloat& maxDepth, const Dfloat& minDepth, const Dint& height)
		{
			//	/*
			//	Dfloat dy = dotNode.y_max - dotNode.y_min;
			//	Dfloat dz = dotNode.max_depth - dotNode.min_depth;
			//	Dfloat rate = (currentHeight - dotNode.y_min ) / dy;
			//	DepthLeft = dotNode.min_depth + ( dz * rate );
			//	*/
			return minDepth + ((maxDepth - minDepth) * ((height - min) / (max - min)));
		}
	};
}

		