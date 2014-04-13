#ifndef _RASTERIZER_H_
#define _RASTERIZER_H_

#include "pipelineElements.h"
#include "Mesh.h"

#include <memory>
#include <vector>
#include <map>
#include <iterator>
#include <vector>
#include <set>
#include <algorithm>

namespace xtozero
{
	class Rect
	{
	public:
		int m_left;
		int m_top;
		int m_right;
		int m_bottom;

		Rect() {}
		Rect( int right, int bottom, int left = 0, int top = 0 )
			: m_right( right ), m_bottom( bottom ), m_left( left ), m_top( top )
		{}
		~Rect( ) {}
	};

	class Edge
	{
	public:
		int m_minY;
		int m_maxY;
		float m_minX;
		float m_maxX;
		float m_startZ;
		float m_endZ;
		float m_gradient;

		Edge( int minY, int maxY, float minX, float maxX, float startZ, float endZ, float gradient )
			: m_minY( minY ), m_maxY( maxY ), m_minX( minX ), m_maxX( maxX ), 
			m_startZ(startZ), m_endZ(endZ), m_gradient( gradient ) {}
		~Edge() {}
	};

	class CRasterizer
	{
	private:
		std::vector<Edge> m_edgeTable;
		std::vector<Edge> m_activeEdgeTable;
		std::vector<CPsElementDesc> m_outputRS;
		Rect m_viewport;

		void CreateEdgeTable( const CRsElementDesc& rsInput, unsigned int faceNumber );
		void UpdateActiveEdgeTable( int scanline );
		float GetIntersectXpos( int minY, int maxY, int scanlineY, float minX, float gradient ) const;
		void ProcessScanline( int scanline, unsigned int facecolor );
	public:
		CRasterizer( void ) {}
		~CRasterizer( void ) {}
		const std::vector<CPsElementDesc>& Process( CRsElementDesc& rsInput );
		const std::vector<CPsElementDesc>& ProcessParallel( CRsElementDesc& rsInput );
		void SetViewPort( int left, int top, int right, int bottom );
	};
}

#endif