#pragma once

#include "../Utility/math3d.h"

#include <vector>

struct WPolyFace
{
	std::vector<int> m_indices;
};

class WPolyData
{
public:
	WPolyData(){}
	~WPolyData()
	{
		Release();
	}

	void SetVertexNum(size_t num)
	{
		m_vertices.reserve(num);
	}
	int GetVertexNum(size_t faceIndex) const
	{
		if (faceIndex >= m_faces.size())
		{
			return 0;
		}
		return m_faces[faceIndex]->m_indices.size();
	}
	void SetFaceNum(int num)
	{
		m_faces.reserve(num);
	}
	int GetFaceNum() const
	{
		return m_faces.size();
	}

	Vector3* GetVertex(size_t faceIndex, size_t vertexIndex)
	{
		if (faceIndex >= m_faces.size())
		{
			return nullptr;
		}
		if (vertexIndex >= m_faces[faceIndex]->m_indices.size())
		{
			return nullptr;
		}
		return m_vertices[m_faces[faceIndex]->m_indices[vertexIndex]];
	}

	const Vector3* GetVertex(size_t faceIndex, size_t vertexIndex) const
	{
		if (faceIndex >= m_faces.size())
		{
			return nullptr;
		}
		if (vertexIndex >= m_faces[faceIndex]->m_indices.size())
		{
			return nullptr;
		}
		return m_vertices[m_faces[faceIndex]->m_indices[vertexIndex]];
	}
	void PushVertex(Vector3* vertex)
	{
		m_vertices.push_back(vertex);
	}

	void PushFace(WPolyFace* face)
	{
		m_faces.push_back(face);
	}

	void Release()
	{
		for (size_t i = 0; i < m_vertices.size(); ++i)
		{
			delete m_vertices[i];
		}
		for (size_t i = 0; i < m_faces.size(); ++i)
		{
			delete m_faces[i];
		}
		m_vertices.clear();
		m_faces.clear();
	}

private:
	std::vector<Vector3*> m_vertices;
	std::vector<WPolyFace*> m_faces;
};