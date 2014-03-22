#pragma once

#include "Mesh.h"

#include <memory>
#include <vector>
#include <map>
#include <iterator>]
#include <vector>

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

		void CreateEdgeTable(const std::shared_ptr<CMesh>& pVertex);
	public:
		explicit CRasterizer(const std::shared_ptr<CMesh> pMesh);
		~CRasterizer(void);
	};
}

