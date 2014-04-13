#ifndef _RASTERIZER_H_
#define _RASTERIZER_H_

#include "pipelineElements.h"
#include "Mesh.h"
#include "XtzThreadPool.h"

#include <memory>
#include <vector>
#include <map>
#include <iterator>
#include <vector>
#include <set>
#include <algorithm>

#define PIXEL_COLOR(r, g, b)  ( ( b << 16 ) + ( g << 8 ) + r )
#define RAND_COLOR() PIXEL_COLOR( ( rand()%255 + 1 ), ( rand()%255 + 1 ), ( rand()%255 + 1 ) )

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

		void CreateEdgeTable( const CRsElementDesc& rsInput, unsigned int faceNumber );
		void UpdateActiveEdgeTable( int scanline );
		void ProcessScanline( int scanline, unsigned int facecolor );

		float GetIntersectXpos( int minY, int maxY, int scanlineY, float minX, float gradient ) const;
	public:
		CRITICAL_SECTION m_cs;
		std::vector<CPsElementDesc> m_outputRS;
		Rect m_viewport;

		CRasterizer( void ) 
		{
			InitializeCriticalSection( &m_cs );
		}
		~CRasterizer( void ) 
		{
			DeleteCriticalSection( &m_cs );
		}
		const std::vector<CPsElementDesc>& Process( CRsElementDesc& rsInput );
		const std::vector<CPsElementDesc>& ProcessParallel( CRsElementDesc& rsInput, CXtzThreadPool* threadPool );

		void CreateEdgeTableParallel( const CRsElementDesc& rsInput, unsigned int faceNumber, 
			std::vector<Edge>& edgeTable );
		void UpdateActiveEdgeTableParallel( int scanline, std::vector<Edge>& edgeTable, std::vector<Edge>& activeEdgeTable );
		void ProcessScanlineParallel( int scanline, unsigned int facecolor, std::vector<Edge>& activeEdgeTable, std::vector<CPsElementDesc>& outputRS );

		void SetViewPort( int left, int top, int right, int bottom );
	};

	struct RsThreadArg
	{
		CRasterizer* pRs;
		CRsElementDesc* pRsElementDesc;
		int index;
	};

	static void rsThreadWork( LPVOID arg )
	{
		RsThreadArg* pRsArg = (RsThreadArg*)arg;

		CRasterizer* rasterizer = pRsArg->pRs;

		std::vector<Edge> edgeTable;
		std::vector<Edge> activeEdgeTable;
		std::vector<CPsElementDesc> outputRS;

		rasterizer->CreateEdgeTableParallel( *pRsArg->pRsElementDesc, pRsArg->index, edgeTable );

		int scanline = edgeTable.begin( )->m_minY;

		unsigned int facecolor = RAND_COLOR( );

		while ( !(edgeTable.empty( ) && activeEdgeTable.empty( )) )
		{
			rasterizer->UpdateActiveEdgeTableParallel( scanline , edgeTable, activeEdgeTable );

			rasterizer->ProcessScanlineParallel( scanline, facecolor, activeEdgeTable, outputRS );

			++scanline;
		}

		EnterCriticalSection( &rasterizer->m_cs );
		for ( std::vector<CPsElementDesc>::iterator& iter = outputRS.begin(); iter != outputRS.end(); ++iter )
		{
			rasterizer->m_outputRS.emplace_back( *iter );
		}
		LeaveCriticalSection( &rasterizer->m_cs );

		delete pRsArg;
	}
}

#endif