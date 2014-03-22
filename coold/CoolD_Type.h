#pragma once

#include <vector>
#include <list>
#include <forward_list>
#include <map>
#include <algorithm>
#include <math.h>

typedef int		Dint;
typedef char	Dchar;
typedef long	Dlong;
typedef float	Dfloat;
typedef double	Ddouble;
typedef unsigned int	Duint;
typedef unsigned char	Duchar;
typedef unsigned long	Dulong;

using namespace std;

struct BaseVertex
{
	Dfloat x, y, z;

	BaseVertex()
	: x(0), y(0), z(0)
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
};

struct EdgeNode
{
	Dfloat x_min;
	Dfloat y_max;
	Dfloat reverseSlope;
};

struct LineKey
{
	Duint beginIndex;
	Duint endIndex;

	LineKey() : beginIndex(-1), endIndex(-1)
	{}

	bool operator==(const LineKey& line) const
	{
		//Line Ÿ�Կ� �ε����� �����ص־��ϳ�.........���߿� ��������� �𸣴� �ϴ� �����Ͽ� ��
		if( this->beginIndex == line.beginIndex && this->endIndex == line.endIndex )
		{
			return true;
		}

		return false;
	}

	bool operator<(const LineKey& lineKey) const	//���� ���̺� ���� Ű������ ����ϱ� ���ؼ� �����ε�
	{
		return beginIndex < lineKey.beginIndex;
	}
};

struct Line
{	
	LineKey		lineKey;	
	BaseVertex	beginVertex;	
	BaseVertex	endVertex;	

	Line() : lineKey()
	{}	
	
	bool operator==(const Line& line) const
	{
		//Line Ÿ�Կ� �ε����� �����ص־��ϳ�.........���߿� ��������� �𸣴� �ϴ� �����Ͽ� ��
		if( lineKey == line.lineKey &&
			this->beginVertex == line.endVertex && this->endVertex   == line.endVertex	)
		{
			return true;
		}

		return false;
	}
};