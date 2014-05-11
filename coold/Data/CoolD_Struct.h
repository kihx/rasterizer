#pragma once
#include "CoolD_Type.h"
#include "..\Math\CoolD_Vector3.h"
#include <random>

struct BaseColor
{	
	Duchar		r, g, b, a;
};

class BaseFace
{
public:
	BaseColor		color;
	vector<Duint>	vecIndex;	//msh 메쉬만 없으면 array<Duint, 3>으로 변경가능	

	BaseFace(){};
	BaseFace(const BaseFace& baseFace) 
		: color(baseFace.color), vecIndex(baseFace.vecIndex){}
};

struct EdgeNode
{
	Dfloat x_min;
	Dfloat y_min;
	Dfloat y_max;
	Dfloat min_depth;	//y_min에서의 깊이값
	Dfloat max_depth;	//y_max에서의 깊이값
	Dfloat reverseSlope;

	EdgeNode() : x_min(0.0f), y_min(0.0f), y_max(0.0f), min_depth(0.0f), max_depth(0.0f), reverseSlope(0.0f){}
	EdgeNode(Dfloat _x_min, Dfloat _y_min, Dfloat _y_max, Dfloat _min_depth, Dfloat _max_depth, Dfloat _reverseSlope)
		:x_min(_x_min), y_min(_y_min), y_max(_y_max), min_depth(_min_depth), max_depth(_max_depth), reverseSlope(_reverseSlope)	{}

	EdgeNode(const EdgeNode& edgeNode)
		:x_min(edgeNode.x_min), y_min(edgeNode.y_min), y_max(edgeNode.y_max), 
		 min_depth(edgeNode.min_depth), max_depth(edgeNode.max_depth), reverseSlope(edgeNode.reverseSlope)	{}	
};

struct LineKey
{
	Dint beginIndex;
	Dint endIndex;

	LineKey()
		: beginIndex(0), endIndex(0) {}

	LineKey(Dint _beginIndex, Dint _endIndex)
		: beginIndex(_beginIndex), endIndex(_endIndex) {}

	LineKey(const LineKey& lineKey)
		:beginIndex(lineKey.beginIndex), endIndex(lineKey.endIndex){}

	Dbool operator==(const LineKey& line) const
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

	LineEdge() = default;

	LineEdge(const LineKey& _lineKey, const EdgeNode& _edgeNode)
		:lineKey(_lineKey), edgeNode(_edgeNode){}

	LineEdge(const LineEdge& lineEdge)
		:lineKey(lineEdge.lineKey), edgeNode(lineEdge.edgeNode){}
};

struct Line
{
	LineKey	lineKey;
	Vector3	beginVertex;
	Vector3	endVertex;
	Dbool	isOneCount;

	Line() = default;
	Line(LineKey _lineKey, Vector3 _beginVertex, Vector3 _endVertex, Dbool _isOneCount = false)
		:lineKey(_lineKey.beginIndex, _lineKey.endIndex), beginVertex(_beginVertex),
		 endVertex(_endVertex), isOneCount(_isOneCount){}

	Line(const Line& _line)
		:lineKey(_line.lineKey), beginVertex(_line.beginVertex), 
		 endVertex(_line.endVertex), isOneCount(_line.isOneCount){}

	Dfloat& GetMinY()
	{
		return (beginVertex.y < endVertex.y) ? beginVertex.y : endVertex.y;
	}

	Dbool operator==(const Line& line) const
	{
		if( lineKey == line.lineKey &&
			this->beginVertex == line.endVertex && 
			this->endVertex == line.endVertex )
		{
			return true;
		}

		return false;
	}
};

struct ActiveLine
{
	Dint height;
	vector<Line> currentLine;

	ActiveLine() = default;
	ActiveLine(const Dint& _height, const vector<Line> _currentLine)
		:height(_height), currentLine(_currentLine)	{}
};

template <typename T>
class RandomGenerator
{ //mt19937
public:
	RandomGenerator(T min, T max)
	{
		static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value, "base type must be integral or floating point");

		random_engine = std::default_random_engine(rd());
		uniform_dist = std::uniform_int_distribution<T>(min, max);
	}

	T GetRand()
	{
		return uniform_dist(random_engine);
	}

private:
	std::random_device rd;
	std::default_random_engine random_engine;
	std::uniform_int_distribution<T> uniform_dist;
};