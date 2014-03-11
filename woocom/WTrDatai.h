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
	WTriData():m_vertexNum(0), m_faceNum(0){};
	~WTriData(){};

	void SetVertexNum( int num )
	{
		m_vertexNum = num;
		m_vertex.reserve( num );
	}
	void SetFaceNum( int num )
	{
		m_faceNum = num;
		m_face.reserve( num );
	}
	void PushVertex( VERTEX* vertex)
	{
		m_vertex.push_back(vertex);
	}

	void PushFace( WFace* face)
	{
		m_face.push_back( face );
	}
		
private:
	std::vector<VERTEX*>	m_vertex;
	std::vector<WFace*>		m_face;

	int m_vertexNum;
	int m_faceNum;
};

