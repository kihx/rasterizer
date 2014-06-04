#ifndef _PIPELINEELEMENTS_H_
#define _PIPELINEELEMENTS_H_

#include "Mesh.h"

#include <map>
#include <vector>

namespace xtozero
{
	class CRsElementDesc
	{
	public:
		CRsElementDesc( ) {}
		~CRsElementDesc( ) {}

		std::vector<Vector4> m_vertices;
		std::vector<Vector3> m_Color;
		std::vector<Vector3> m_localVertices;
		std::vector<std::vector<int>> m_faces;
		Vector4 m_cameraPos;
		COORDINATE m_coordinate;
	};

	class CPsElementDesc
	{
	public:
		CPsElementDesc() {}
		CPsElementDesc( const CPsElementDesc& element ) 
			: m_x( element.m_x ), m_y( element.m_y ), m_z( element.m_z ), m_color( element.m_color ) {}
		CPsElementDesc( const int x, const int y, const float z, const unsigned int color, const Vector3& localVertex )
			: m_x( x ), m_y( y ), m_z( z ), m_color( color ), m_localVertex( localVertex ) {}
		~CPsElementDesc( ) {}

		int m_x;
		int m_y;
		float m_z;
		unsigned int m_color;
		Vector3 m_localVertex;
	};

	class COmElementDesc
	{
	public:
		COmElementDesc() {}
		COmElementDesc( const COmElementDesc& element )
			: m_x( element.m_x ), m_y( element.m_y ), m_z( element.m_z ), m_color( element.m_color ) {}
		COmElementDesc( const int x, const int y, const float z, const unsigned int color )
			: m_x( x ), m_y( y ), m_z( z ), m_color( color ) {}
		~COmElementDesc( ) {}

		int m_x;
		int m_y;
		float m_z; 
		unsigned int m_color;
	};
}

#endif