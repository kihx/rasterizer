#include "stdafx.h"
#include "Rasterizer.h"

namespace xtozero
{
	CRasterizer::CRasterizer(std::shared_ptr<CMesh> pMesh)
	{
		//���⿡�� �޽ó����� �ȼ��� ���


	}	

	CRasterizer::~CRasterizer(void)
	{

	}

	std::pair<int, int> CreateEdgeTable(const std::vector<Vertex<float>>& vertexlist)
	{
		//To Do : EdgeTable�� ����� minY MaxY�� ��ȯ�� ����
		int max = -INT_MAX;
		int min = INT_MAX;

		for( auto iter = vertexlist.begin(); iter != vertexlist.end(); ++iter )
		{
			if( iter->m_element[y] > max )
			{
				max = static_cast<int>(iter->m_element[y]);
			}
			if( iter->m_element[y] < min )
			{
				min = static_cast<int>(iter->m_element[y]);
			}
		}

		return std::pair<int, int>(min, max);
	}
}