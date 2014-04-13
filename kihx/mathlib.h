#pragma once

#include "base.h"
#include <intrin.h>


namespace kih
{
	FORCEINLINE float ToRadian( float degree )
	{
		return degree * ( PI / 180.0f );
	}

	FORCEINLINE float ToDegree( float radian )
	{
		return radian * ( 180.0f / PI );
	}

	template<class T>
	FORCEINLINE const T& Min( const T& lhs, const T& rhs )
	{
		static_assert( std::is_integral<T>::value || std::is_floating_point<T>::value, "base type must be integral or floating point" );
		return ( lhs <= rhs ) ? lhs : rhs;
	}

	template<class T>
	FORCEINLINE const T& Max( const T& lhs, const T& rhs )
	{
		static_assert( std::is_integral<T>::value || std::is_floating_point<T>::value, "base type must be integral or floating point" );
		return ( lhs >= rhs ) ? lhs : rhs;
	}

	template<class T>
	FORCEINLINE const T& Clamp( const T& value, const T& min, const T& max )
	{
		static_assert( std::is_integral<T>::value || std::is_floating_point<T>::value, "base type must be integral or floating point" );
		return Max<T>( Min<T>( value, max ), min );
	}

	FORCEINLINE int Trunc( float f )
	{
		return _mm_cvtt_ss2si( _mm_set_ss( f ) );
	}

	FORCEINLINE int Ceil( float f )
	{
		return _mm_cvt_ss2si( _mm_ceil_ps( _mm_set_ss( f ) ) );
	}

	FORCEINLINE int Floor( float f )
	{
		return _mm_cvt_ss2si( _mm_floor_ps( _mm_set_ss( f ) ) );
	}

	FORCEINLINE int Round( float f )
	{
		return _mm_cvt_ss2si( _mm_round_ps( _mm_set_ss( f ), _MM_FROUND_NINT ) );
	}

	FORCEINLINE float Byte_ToFloatNormalized( byte b )
	{
		const float Converter = 1.0f / 255.0f;
		return Clamp<float>( static_cast< float >( b * Converter ), 0.0f, 1.0f );
	}

	FORCEINLINE byte Float_ToByte( float f )
	{
		return static_cast< byte >( Clamp<float>( f, 0.0f, 1.0f ) * 255.0f );
	}

	template<class T>
	FORCEINLINE T Float_ToInteger( float f )
	{
		static_assert( std::is_integral<T>::value, "return type must be integer" );
		return static_cast< T >( Trunc( f ) );
	}

	FORCEINLINE float Lerp( float a, float b, float ratio )
	{
		return a + ( b - a ) * ratio;
	}
};
