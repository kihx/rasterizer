#pragma once

#include "Mesh.h"

#include <memory>
#include <vector>
#include <map>
#include <iterator>
#include <vector>
#include <set>

namespace xtozero
{
	class Edge
	{
	public:
		int m_maxY;
		float m_minX;
		float m_gradient;

		Edge(int maxY, float minX, float gradient) : m_maxY(maxY), m_minX(minX), m_gradient(gradient) {}
		~Edge() {}
	};

	class CRasterizer
	{
	private:
		std::vector<Vertex<int>> m_pixels;
		std::map<int, std::vector<Edge>> m_edgeTable;
		std::vector<Edge> m_activeEdgeTable;

		void CreateEdgeTable(const std::shared_ptr<CMesh>& pVertex);
		void UpdateActiveEdgeTable(const int scanline);
		float GetIntersectXpos(const int minY, const int scanlineY, const float minX, const float gradient) const;
	public:
		explicit CRasterizer(const std::shared_ptr<CMesh> pMesh);
		~CRasterizer(void);
	};
}

