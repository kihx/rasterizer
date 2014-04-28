#pragma once

#include <array>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#include <cassert>
#include <chrono>
#include <tuple>
#include <ppl.h>
#include <mutex>
#include <iostream>

typedef void	Dvoid;
typedef bool	Dbool;
typedef int		Dint;
typedef char	Dchar;
typedef long	Dlong;
typedef float	Dfloat;
typedef double	Ddouble;
typedef unsigned int	Duint;
typedef unsigned char	Duchar;
typedef unsigned long	Dulong;

using namespace std;
using namespace Concurrency;
typedef chrono::system_clock::time_point TimeForm;

enum TransType
{
	WORLD,
	VIEW,
	PERSPECTIVE,
	VIEWPORT,	
	TRANSFORM_END
};

enum MeshType
{
	MSH,
	PLY,
	MESHTYPE_END
};

enum class BSCULL
{
	CW,
	CCW,
	ALL
};