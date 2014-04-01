#pragma once
#include "CoolD_Type.h"
#include "CoolD_Vector3.h"
#include <random>

typedef Vector3 BaseVertex;

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
		:color(baseFace.color), vecIndex(baseFace.vecIndex)
	{}
};

struct EdgeNode
{
	Dfloat x_min;
	Dfloat y_max;
	Dfloat reverseSlope;

	EdgeNode() : x_min(0.0f), y_max(0.0f), reverseSlope(0.0f){}

	EdgeNode(Dfloat _x_min, Dfloat _y_max, Dfloat _reverseSlope)
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

	LineKey(Duint _beginIndex, Duint _endIndex)
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

	Line(LineKey _lineKey, BaseVertex _beginVertex, BaseVertex _endVertex)
		:lineKey(_lineKey.beginIndex, _lineKey.endIndex), beginVertex(_beginVertex), endVertex(_endVertex) {}

	Line(const Line& _line)
		:lineKey(_line.lineKey), beginVertex(_line.beginVertex), endVertex(_line.endVertex) {}

	Dfloat& GetMinY()
	{
		return (beginVertex.y < endVertex.y) ? beginVertex.y : endVertex.y;
	}

	Dbool operator==(const Line& line) const
	{
		//Line 타입에 인덱스를 저장해둬야하나.........나중에 써먹을지도 모르니 일단 포함하여 비교
		if( lineKey == line.lineKey &&
			this->beginVertex == line.endVertex && this->endVertex == line.endVertex )
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

	ActiveLine(const Dint& _height, const list<Line> _currentLine)
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