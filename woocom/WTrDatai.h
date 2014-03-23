#pragma once

#include <vector>

template <typename T>
class WVertex
{
public:
	T	m_pos[3];
};

class WFace
{
public:
	std::vector<int>	m_index;
	unsigned char	m_rgb[3];
};

typedef WVertex<float> VERTEX;

class WTriData
{
public:
	WTriData(){}
	~WTriData()
	{
		Release();
	}

	void SetVertexNum( size_t num )
	{
		m_vertices.reserve( num );
	}
	int GetVertexNum(size_t faceIndex) const
	{ 
		if( faceIndex >= m_faces.size())
		{
			return 0;
		}
		return m_faces[faceIndex]->m_index.size(); 
	}
	void SetFaceNum( int num )
	{
		m_faces.reserve( num );
	}
	int GetFaceNum() const
	{
		return m_faces.size();
	}

	const unsigned char* GetFaceColor(size_t index) const
	{
		if( index >= m_faces.size() )
		{
			return nullptr;
		}
		return m_faces[index]->m_rgb;
	}

	const VERTEX* GetVertex(size_t faceIndex, size_t vertexIndex) const
	{
		if( faceIndex >= m_faces.size())
		{
			return nullptr;
		}
		if( vertexIndex >= m_faces[faceIndex]->m_index.size() )
		{
			return nullptr;
		}
		return m_vertices[ m_faces[faceIndex]->m_index[vertexIndex] ];
	}
	void PushVertex( VERTEX* vertex)
	{
		m_vertices.push_back(vertex);
	}

	void PushFace( WFace* face)
	{
		m_faces.push_back( face );
	}

	void Release()
	{
		for(size_t i=0; i < m_vertices.size(); ++i )
		{
			delete m_vertices[i];
		}
		for(size_t i=0; i < m_faces.size(); ++i )
		{
			delete m_faces[i];
		}
		m_vertices.clear();
		m_faces.clear();
	}
		
private:
	std::vector<VERTEX*>	m_vertices;
	std::vector<WFace*>		m_faces;
};

