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

//���� ��ȯ �ε���
enum TransType
{
	WORLD,
	VIEW,
	PERSPECTIVE,
	VIEWPORT,	
	TRANSFORM_END
};

//MSH : ����, ��, ���ٱ��� �̹� ���� �Ǿ� �ִ� Ÿ��
//PLY : ���� ��ǥ �޽� Ÿ��
enum MeshType
{
	MSH,
	PLY,
	MESHTYPE_END
};

//BackSpace �ø� ����
enum class BSCullType
{
	CW,		//�ð�������� �׷��� Face �ø�
	CCW,	//�ݽð�
	ALL
};

//Thread �۾��� �����ϴ� EventH
enum EventType
{
	WORK,		//�׸��� �۾�
	SHUTDOWN,	//���� �۾�
	TYPECOUNT
};

using Func_void_initList = void(*)(initializer_list<string>);	//c++11 using syntax
