#ifndef _PIPELINEELEMENTS_H_
#define _PIPELINEELEMENTS_H_

#include "XtzMath.h"
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
		std::vector<std::vector<int>> m_faces;
		COORDINATE m_coodinate;
	};

	class CPsElementDesc
	{
	public:
		CPsElementDesc() {}
		CPsElementDesc( const CPsElementDesc& element ) 
			: m_x( element.m_x ), m_y( element.m_y ), m_z( element.m_z ), m_color( element.m_color ) {}
		CPsElementDesc( const int& x, const int& y, const float& z, const unsigned int& color )
			: m_x( x ), m_y( y ), m_z( z ), m_color( color ) {}
		~CPsElementDesc( ) {}

		int m_x;
		int m_y;
		float m_z;
		unsigned int m_color;
	};

	class COmElementDesc
	{
	public:
		COmElementDesc() {}
		COmElementDesc( const COmElementDesc& element )
			: m_x( element.m_x ), m_y( element.m_y ), m_z( element.m_z ), m_color( element.m_color ) {}
		COmElementDesc( const int& x, const int& y, const float& z, const unsigned int& color )
			: m_x( x ), m_y( y ), m_z( z ), m_color( color ) {}
		~COmElementDesc( ) {}

		int m_x;
		int m_y;
		float m_z; 
		unsigned int m_color;
	};
}

#endif