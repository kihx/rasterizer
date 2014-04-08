#pragma once

#include "..\Data\CoolD_Type.h"

#define kEpsilon    1.0e-6f

#define kPI         3.1415926535897932384626433832795f
#define kHalfPI     1.5707963267948966192313216916398f
#define kTwoPI      2.0f*kPI

inline Dfloat Sqrt( Dfloat val )        { return sqrtf( val ); }
inline Dfloat InvSqrt( Dfloat val )     { return 1.0f/sqrtf( val ); }
inline Dfloat Abs( Dfloat f )           { return fabsf(f); }

extern Dvoid FastSinCos( Dfloat a, Dfloat& sina, Dfloat& cosa );

inline Dbool IsZero( Dfloat a )				{    return ( fabsf(a) < kEpsilon );		}
inline Dbool AreEqual( Dfloat a, Dfloat b )	{    return ( ::IsZero(a-b) );				}
inline Dfloat Sin( Dfloat a )				{    return sinf(a);						}
inline Dfloat Cos(Dfloat a)					{	 return cosf(a);						}
inline Dfloat Tan( Dfloat a )				{    return tanf(a);						}

inline Dvoid SinCos( Dfloat a, Dfloat& sina, Dfloat& cosa )
{    
	sina = sinf(a);    
	cosa = cosf(a);
}