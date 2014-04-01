#pragma once

#include <intrin.h>
#include <type_traits>


#pragma warning( disable: 4201 )	// warning C4201: nonstandard extension used : nameless struct/union



namespace kih
{
	// type definitions
	//
	typedef unsigned char byte;
	

	// known constants
	//
	const float PI = 3.141592654f;


	// useful functions
	//
	inline float ToRadian( float degree )
	{
		return degree * ( PI / 180.0f );
	}

	inline float ToDegree( float radian )
	{
		return radian * ( 180.0f / PI );
	}

	template<typename T>
	inline const T& Min( const T& lhs, const T& rhs )
	{
		static_assert( std::is_integral<T>::value || std::is_floating_point<T>::value, "base type must be integral or floating point" );
		return ( lhs <= rhs ) ? lhs : rhs;
	}

	template<typename T>
	inline const T& Max( const T& lhs, const T& rhs )
	{
		static_assert( std::is_integral<T>::value || std::is_floating_point<T>::value, "base type must be integral or floating point" );
		return ( lhs >= rhs ) ? lhs : rhs;
	}

	template<typename T>
	inline const T& Clamp( const T& value, const T& min, const T& max )
	{
		static_assert( std::is_integral<T>::value || std::is_floating_point<T>::value, "base type must be integral or floating point" );
		return Max<T>( Min<T>( value, max ), min );
	}

	inline int Trunc( float f )
	{
		return _mm_cvtt_ss2si( _mm_set_ss( f ) );
	}

	inline float Byte_ToFloatNormalized( byte b )
	{
		const float Converter = 1.0f / 255.0f;
		return Clamp<float>( static_cast< float >( b * Converter ), 0.0f, 1.0f );
	}

	inline byte Float_ToByte( float f )
	{
		return static_cast< byte >( Clamp<float>( f, 0.0f, 1.0f ) * 255.0f );
	}

	template<typename T>
	inline T Float_ToInteger( float f )
	{
		static_assert( std::is_integral<T>::value, "return type must be integer" );
		return static_cast<T>( Trunc( f ) );
	}

	inline float Lerp( float a, float b, float ratio )
	{
		return a + ( b - a ) * ratio;
	}
};
