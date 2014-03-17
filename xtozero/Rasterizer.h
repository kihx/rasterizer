#pragma once

#include "Mesh.h"

#include <memory>
#include <vector>

namespace xtozero
{
	struct Edge
	{
		int m_maxY;
		int m_minX;
		float m_gradient;
	};

	class CRasterizer
	{
	private:
		std::vector<Vertex<int>> m_pixels;

		std::pair<int, int>& GetMinMaxY(const std::vector<Vertex<float>>& pVertex) const;
	public:
		explicit CRasterizer(std::shared_ptr<CMesh> pMesh);
		~CRasterizer(void);
	};
}

