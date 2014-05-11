#pragma once

#include "..\Data\CoolD_Defines.h"
#include "..\Data\CoolD_Type.h"
#include "..\Data\CoolD_Struct.h"
#include "..\Math\CoolD_Matrix44.h"
#include <concurrent_vector.h>
#include "..\Data\CoolD_Singleton.h"

namespace CoolD
{
	class CustomMesh;
	class DepthBuffer;
	class RenderModule final	//상속 받을일은 없을 듯
	{
	private:
		Duchar* m_Buffer;	//프레임 버퍼
		DepthBuffer* m_pDepthBuffer;
		Dint m_Width;
		Dint m_Height;
		BSCullType m_CullMode;
		
		vector<Line> m_vecLine;
		vector<LineEdge> m_edgeTable;
		vector<ActiveLine> m_activeTable;		
		array<Matrix44, TRANSFORM_END> m_arrayTransform;	

	private:
		vector<Vector3>	m_trasnformVertex;
		CustomMesh* m_pMesh;		

	public:
		RenderModule();		
		~RenderModule();
				
	public:
		Dvoid RenderBegin(CustomMesh* pMesh);
		Dvoid RenderEnd();
		
	private:
		Dvoid AdjustTransform();
		Dvoid Render();

	public:
		Dvoid ClearColorBuffer(Dulong clearColor);
		void SetScreenInfo(Dvoid* buffer, const int width, const int height);
		inline void SetTransform(const TransType& type, const Matrix44& matrix)	{ m_arrayTransform[ type ] = matrix; }		

	private:	//렌더 관련 데이터 생성부			
		Dvoid CreatePointsToLines(const vector<BaseFace>* pvecFace, Duint faceNum, vector<Line>& vecLine);
		Dvoid CreateEdgeTable(vector<Line>& vecLine, vector<LineEdge>& edgeTable);
		Dvoid CreateChainTable(vector<Line>& vecLine, vector<ActiveLine>& activeTable);

	private:	//컬링
		Dbool BackSpaceCulling(vector<Line>& vecLine);
		Dbool FrustumCulling(const BaseFace& pCurrentface);

	public:
		inline Dvoid SetBackSpaceCullType(BSCullType type){ m_CullMode = type; }

	private:	//그려지는 단계
		Dvoid DrawDot(const Duint& x, const Duint& y, const Dulong& DotColor);
		Dvoid DrawLine(vector<EdgeNode>& renderLine, const Dint currentHeight, const Dulong dotColor);
		Dvoid DrawFace(vector<ActiveLine>& activeTable, vector<LineEdge>& edgeTable, const Dulong dotColor);
		
	private:	//그 외 보조 함수				
		Dvoid InsertPointToLine(vector<Line>& vecLine, const BaseFace& face, Duint firstIndex, Duint secondIndex);
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

	class DepthBuffer : public CSingleton<DepthBuffer>
	{
		friend class CSingleton<DepthBuffer>;

	private:
		DepthBuffer();
		DepthBuffer(const DepthBuffer&) = delete;
		DepthBuffer& operator=(const DepthBuffer&) = delete;

	public:
		virtual	~DepthBuffer();

	public:
		void ClearDepthBuffer(Dint height, Dint width);
		Dbool DepthTest(Dint x, Dint y, Dfloat depth);

	private:		
		Dint	m_width;
		Dfloat* m_DepthBuffer;	//깊이 버퍼
	};
}

		