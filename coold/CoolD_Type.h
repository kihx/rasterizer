#pragma once

#include <vector>
#include <list>
#include <vector>
#include <map>
#include <algorithm>
#include <math.h>
#include <assert.h>
#include <chrono>

typedef int		Dint;
typedef char	Dchar;
typedef long	Dlong;
typedef float	Dfloat;
typedef double	Ddouble;
typedef unsigned int	Duint;
typedef unsigned char	Duchar;
typedef unsigned long	Dulong;

typedef Duchar Buffer;



using namespace std;
typedef chrono::system_clock::time_point TimeForm;
struct BaseVertex
{
	Dfloat x, y, z;

	BaseVertex()
		: x(0), y(0), z(0)
	{}

	BaseVertex(const BaseVertex& baseVector) 
		:x(baseVector.x), y(baseVector.y), z(baseVector.z)
	{}

	bool operator==(const BaseVertex& vetex) const
	{
		if( this->x == vetex.x &&
			this->y == vetex.y &&
			this->z == vetex.z )
		{
			return true;
		}
		return false;
	}

	bool operator!=(const BaseVertex& vetex) const
	{
		if( this->x != vetex.x ||
			this->y != vetex.y ||
			this->z != vetex.z )
		{
			return true;
		}
		return false;
	}
};

struct BaseColor
{
	Duchar		r, g, b, a;
};

struct BaseFace
{
	BaseColor		color;
	vector<Dint>	vecIndex;

	BaseFace() = default;
	BaseFace(const BaseFace& baseFace)
		:color(baseFace.color), vecIndex( baseFace.vecIndex )
	{}
};

struct EdgeNode
{
	Dfloat x_min;
	Dfloat y_max;
	Dfloat reverseSlope;
	
	EdgeNode() : x_min(0.0f), y_max(0.0f), reverseSlope(0.0f){}

	EdgeNode( Dfloat _x_min, Dfloat _y_max, Dfloat _reverseSlope ) 
	:x_min(_x_min), y_max(_y_max), reverseSlope(_reverseSlope)	{}

	EdgeNode(const EdgeNode& edgeNode) 
		:x_min(edgeNode.x_min), y_max(edgeNode.y_max), reverseSlope(edgeNode.reverseSlope)	{}	
};

struct LineKey
{
	Duint beginIndex;
	Duint endIndex;

	LineKey() 
	: beginIndex(-1), endIndex(-1) {}

	LineKey( Duint _beginIndex, Duint _endIndex)
		: beginIndex(_beginIndex), endIndex(_endIndex) {}
	
	LineKey(const LineKey& lineKey)
		:beginIndex(lineKey.beginIndex), endIndex(lineKey.endIndex){}

	bool operator==(const LineKey& line) const
	{		
		if( this->beginIndex == line.beginIndex && 
			this->endIndex == line.endIndex )
		{
			return true;
		}

		return false;
	}	
};

struct LineEdge
{
	LineKey	lineKey;
	EdgeNode edgeNode;		
	
	LineEdge(const LineKey& _lineKey, const EdgeNode& _edgeNode)
		:lineKey(_lineKey), edgeNode(_edgeNode){}

	LineEdge(const LineEdge& lineEdge)
		:lineKey(lineEdge.lineKey), edgeNode(lineEdge.edgeNode){}
};

struct Line
{	
	LineKey		lineKey;	
	BaseVertex	beginVertex;	
	BaseVertex	endVertex;	
		
	Line( LineKey _lineKey, BaseVertex _beginVertex, BaseVertex _endVertex )
		:lineKey( _lineKey.beginIndex, _lineKey.endIndex ), beginVertex(_beginVertex), endVertex(_endVertex) {}

	Line(const Line& _line)
		:lineKey(_line.lineKey), beginVertex(_line.beginVertex), endVertex(_line.endVertex) {}

	Dfloat& GetMinY()
	{
		return (beginVertex.y < endVertex.y)? beginVertex.y : endVertex.y;
	}

	bool operator==(const Line& line) const
	{
		//Line 타입에 인덱스를 저장해둬야하나.........나중에 써먹을지도 모르니 일단 포함하여 비교
		if( lineKey == line.lineKey &&
			this->beginVertex == line.endVertex && this->endVertex   == line.endVertex	)
		{
			return true;
		}

		return false;
	}
};

struct ActiveLine
{
	Dint height;
	list<Line> currentLine;

	ActiveLine(const Dint& _height,	const list<Line> _currentLine )
	:height(_height), currentLine(_currentLine)	{}
};

