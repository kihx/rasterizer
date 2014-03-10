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
	WTriData();
	~WTriData();
		
private:
	VERTEX*	m_vertex;
	WFace* m_face;

	int m_vertexNum;
	int m_faceNum;
};

