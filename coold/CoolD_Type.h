#pragma once

#include <vector>
#include <array>
#include <list>
#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <cmath>
#include <cassert>
#include <chrono>

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
typedef chrono::system_clock::time_point TimeForm;

enum TransType
{
	WORLD,
	VIEW,
	PERSPECTIVE,
	VIEWPORT,
	END
};

enum class MeshType{ MSH, PLY };