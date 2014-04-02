#pragma once

#include <intrin.h>
#include <type_traits>


#pragma warning( disable: 4201 )	// warning C4201: nonstandard extension used : nameless struct/union


#ifndef FORCEINLINE
#define FORCEINLINE		__forceinline
#endif


namespace kih
{
	// type definitions
	//
	typedef unsigned char byte;


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

	FORCEINLINE PrimitiveType GetPrimitiveTypeFromNumberOfVertices( size_t num )
	{
		return static_cast< PrimitiveType >( num );
	}

	FORCEINLINE size_t GetNumberOfVerticesPerPrimitive( PrimitiveType type )
	{	
		return static_cast< size_t >( type );
	}


	/* enum class CoordinateType
	*/
	enum class CoordinatesType
	{
		Projective = 0,
		ReciprocalHomogeneous,
	};
	

	// known constants
	//
	const float PI = 3.141592654f;
};

using kih::PrimitiveType;
using kih::CoordinatesType;
