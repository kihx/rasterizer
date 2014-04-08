#pragma once

#include "..\Data\CoolD_Defines.h"
#include "..\Data\CoolD_Type.h"
#include "..\Data\CoolD_Struct.h"
#include "..\Math\CoolD_Matrix44.h"

namespace CoolD
{
	class CustomMesh;
	class AreaFilling final	//��� �������� ���� ��
	{
	private:
		Duchar* m_Buffer;	//������ ����
		Dfloat* m_DepthBuffer;	//���� ����
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

	private:	//���� ���� ������ ������			
		Dvoid CreatePointsToLines(tuple_meshInfo meshInfo, Duint faceNum);
		Dvoid CreateEdgeTable();
		Dvoid CreateChainTable( );	

	private:
		Dbool DepthTest(const Duint x, const Duint y, Dfloat depth);
	private:	//�׷����� �ܰ�
		Dvoid DrawDot(const Duint x, const Duint y, const Dulong DotColor);
		Dvoid DrawLine(list<EdgeNode>& renderLine, const Dint currentHeight, const Dulong dotColor);
		Dvoid DrawFace(const Dulong dotColor);

	private:	//�� �� ���� �Լ�
		Dulong MixDotColor( const BaseColor& color );
		Dbool CheckContinueLine( const LineKey& lhs, const LineKey& rhs ) const;		
	};
}