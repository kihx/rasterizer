#ifndef _RASTERIZER_H_
#define _RASTERIZER_H_

#include "pipelineElements.h"
#include "Mesh.h"
#include "XtzThreadPool.h"
#include "Concommand.h"

#include <memory>
#include <vector>
#include <map>
#include <iterator>
#include <vector>
#include <set>
#include <algorithm>
#include <tuple>

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
		Vector3 m_startLocalVertex;
		Vector3 m_endLocalVertex;

		Edge( int minY, int maxY, float minX, float maxX, float startZ, float endZ, float gradient,
			const Vector3& startLocalVertex, const Vector3& endLocalVertex )
			: m_minY( minY ), m_maxY( maxY ), m_minX( minX ), m_maxX( maxX ), 
			m_startZ( startZ ), m_endZ( endZ ), m_gradient( gradient ),
			m_startLocalVertex( startLocalVertex ), m_endLocalVertex( endLocalVertex ) {}
		~Edge() {}
	};

	class CRasterizer
	{
	private:
		std::vector<Edge> m_edgeTable;
		std::vector<Edge> m_activeEdgeTable;
		std::vector<std::tuple<int, float, Vector3>> m_horizontalLine;
		SpinLock m_lockobject;
		Rect m_viewport;

		void CreateEdgeTable( const CRsElementDesc& rsInput, unsigned int faceNumber );
		void UpdateActiveEdgeTable( int scanline );
		void ProcessScanline( int scanline, unsigned int facecolor );

		inline float GetIntersectXpos( int minY, int maxY, int scanlineY, float minX, float gradient ) const;
	public:
		std::vector<CPsElementDesc> m_outputRS;

		CRasterizer( void ) 
		{
		}
		~CRasterizer( void ) 
		{
		}
		const std::vector<CPsElementDesc>& Process( CRsElementDesc& rsInput );
		const std::vector<CPsElementDesc>& ProcessParallel( CRsElementDesc& rsInput, CXtzThreadPool* threadPool );
		const std::vector<CPsElementDesc>& ProcessFaceRange( CRsElementDesc& rsInput, unsigned int startface, unsigned int endface );

		void CreateEdgeTableParallel( const CRsElementDesc& rsInput, unsigned int faceNumber, 
			std::vector<Edge>& edgeTable );
		void UpdateActiveEdgeTableParallel( int scanline, std::vector<Edge>& edgeTable, std::vector<Edge>& activeEdgeTable );
		void ProcessScanlineParallel( int scanline, unsigned int facecolor, std::vector<Edge>& activeEdgeTable, 
			std::vector<CPsElementDesc>& outputRS, std::vector<std::tuple<int, float, Vector3>>& horizontalLine );

		void SetViewPort( int left, int top, int right, int bottom );

		SpinLock& GetLockObject()
		{
			return m_lockobject;
		}
		Rect& GetViewport()
		{
			return m_viewport;
		}
	};

	class CBarycentricRasterizer
	{
	private:
		Rect m_viewport;
	public:
		std::vector<CPsElementDesc> m_outputRS;

		CBarycentricRasterizer( void ) {};
		~CBarycentricRasterizer( void ) {};

		const std::vector<CPsElementDesc>& Process( CRsElementDesc& rsInput );
		const std::vector<CPsElementDesc>& ProcessFaceRange( CRsElementDesc& rsInput, unsigned int startface, unsigned int endface );
		float CalcParallelogramArea( const Vector4& p, const Vector4& v0, const Vector4 v1 );

		void SetViewPort( int left, int top, int right, int bottom );

		Rect& GetViewport( )
		{
			return m_viewport;
		}
	};

	struct RsThreadArg
	{
		CRasterizer* pRs;
		CRsElementDesc* pRsElementDesc;
		unsigned int startIdx;
		unsigned int endIdx;
	};

	static void RsThreadWork( LPVOID arg );

	bool IsBackFace( const CRsElementDesc& rsInput, const int facenumber );

	inline bool CompareEdgeY( const Edge& lhs, const Edge& rhs )
	{
		return lhs.m_minY < rhs.m_minY;
	}

	inline bool CompareEdgeX( const Edge& lhs, const Edge& rhs )
	{
		return lhs.m_minX < rhs.m_minX;
	}
}

#endif