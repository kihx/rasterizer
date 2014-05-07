#pragma once

#include <intrin.h>
#include <type_traits>


#pragma warning( disable: 4201 )	// warning C4201: nonstandard extension used : nameless struct/union


#ifndef FORCEINLINE
#define FORCEINLINE		__forceinline
#endif


// type definitions
//
typedef unsigned char byte;
typedef unsigned int bitflags;
typedef __int64 int64;


namespace kih
{
	/* struct IntFloat
	*/
	struct IntFloat
	{
		union
		{
			int I;
			float F;
		};

		IntFloat( int i ) : I( i ) {}
		IntFloat( float f ) : F( f ) {}
	};


	/* enum class PrimitiveType
	*/
	enum class PrimitiveType : unsigned int
	{
		Undefined = 0,
		Points = 1,
		Lines = 2,
		Triangles = 3,
		Quads = 4,
		Pentagons = 5,
		Octas = 6
	};


	/* enum class CoordinateType
	*/
	enum class CoordinatesType
	{
		Projective = 0,
		ReciprocalHomogeneous,
	};


	/* enum ColorFormat
	*/
	enum class ColorFormat : unsigned int
	{
		Unknown = 0,

		// color
		R8G8B8 = 10,

		// depth-stencil
		D8S24 = 1000,	// depth 8 and stencil 24 bits
		D32F,			// depth 32 bits floating point
	};

		
	/* enum class DepthFunc
	*/
	enum class DepthFunc
	{
		None = 0,
		Not,
		Equal,
		Less,
		LessEqual,
		Greater,
		GreaterEqual,
		/* the number of enum elements */
		Size
	};
	

	/* enum class CullMode
	*/
	enum class CullMode
	{
		None = 0,
		CW,
		CCW,
	};


	// known constants
	//
	const float PI = 3.141592654f;
};

using kih::PrimitiveType;
using kih::CoordinatesType;

