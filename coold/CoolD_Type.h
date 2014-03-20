#pragma once

#include <vector>
#include <list>
#include <forward_list>
#include <map>
#include <algorithm>

typedef int		Dint;
typedef char	Dchar;
typedef long	Dlong;
typedef float	Dfloat;
typedef double	Ddouble;
typedef unsigned int	Duint;
typedef unsigned char	Duchar;
typedef unsigned long	Dulong;

using namespace std;

struct baseVertex
{
	Dfloat x, y, z;

	baseVertex()
	: x(0), y(0), z(0)
	{}

	bool operator==(const baseVertex& vetex) const
	{
		if( this->x == vetex.x &&
			this->y == vetex.y &&
			this->z == vetex.z )
		{
			return true;
		}
		return false;
	}

	bool operator!=(const baseVertex& vetex) const
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

struct baseFace
{
	Duchar		r, g, b, a;
	vector<Dint>	vecIndex;
};

struct AreaFillingNode
{
	Dfloat x_min;
	Dfloat y_max;
	Dfloat increment;
};

struct Line
{	
	Duint beginIndex;
	baseVertex beginVertex;

	Duint endIndex;
	baseVertex endVertex;	

	Line() 
	: beginIndex(-1),
	endIndex(-1)	
	{}	
	
	bool operator==(const Line& line) const
	{
		//Line 타입에 인덱스를 저장해둬야하나.........나중에 써먹을지도 모르니 일단 포함하여 비교
		if( this->beginIndex == line.beginIndex && this->beginVertex == line.endVertex && 
			this->endIndex == line.endIndex && this->endVertex   == line.endVertex	)
		{
			return true;
		}

		return false;
	}
};

struct X_LineSort_Desc
{
	//음.... 조건이 맞을라나...
	bool operator()( const Line* pObj1, const Line* pObj2 ) const
	{
		if(pObj1->beginVertex.x <= pObj2->beginVertex.x	)
		{
			if(pObj1->endVertex.x > pObj2->endVertex.x	)
				return false;
		}
		else
		{
			if( pObj1->beginVertex.x > pObj2->beginVertex.x )
				return false;
		}
		return true;
	}	
};
