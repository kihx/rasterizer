#pragma once

#include <array>
#include <vector>
#include <queue>
#include <map>
#include <algorithm>
#include <cmath>
#include <cassert>
#include <chrono>
#include <tuple>
#include <ppl.h>
#include <mutex>
#include <iostream>
#include <string>
#include <initializer_list>
#include <thread>

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
using namespace Concurrency;	//ppl namespace

//공간 변환 인덱스
enum TransType
{
	WORLD,
	VIEW,
	PERSPECTIVE,
	VIEWPORT,	
	TRANSFORM_END
};

//MSH : 월드, 뷰, 원근까지 이미 적용 되어 있는 타입
//PLY : 로컬 좌표 메쉬 타입
enum MeshType
{
	MSH,
	PLY,
	MESHTYPE_END
};

//BackSpace 컬링 종류
enum class BSCullType
{
	CW,		//시계방향으로 그려진 Face 컬링
	CCW,	//반시계
	ALL
};

//Thread 작업을 결정하는 EventH
enum EventType
{
	WORK,		//그리는 작업
	SHUTDOWN,	//종료 작업
	TYPECOUNT
};

using Func_void_initList = void(*)(initializer_list<string>);	//c++11 using syntax
