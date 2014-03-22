#pragma once

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
	class Edge
	{
	public:
		int m_minY;
		int m_maxY;
		float m_minX;
		float m_gradient;

		Edge(int minY, int maxY, float minX, float gradient) 
			: m_minY(minY), m_maxY(maxY), m_minX(minX), m_gradient(gradient) {}
		~Edge() {}
	};

	class CRasterizer
	{
	private:
		std::vector<Vertex<int>> m_pixels;
		std::map<int, std::vector<Edge>> m_edgeTable;
		std::vector<Edge> m_activeEdgeTable;
		std::vector<PS_ELEMENT_DESC> m_outputRS;

		void CreateEdgeTable(const std::shared_ptr<CMesh> pVertex, int faceNumber);
		void UpdateActiveEdgeTable(int scanline);
		float GetIntersectXpos( int minY, int scanlineY, float minX, float gradient) const;
		void ProcessScanline();
	public:
		explicit CRasterizer(const std::shared_ptr<CMesh> pMesh);
		~CRasterizer(void);
	};
}

