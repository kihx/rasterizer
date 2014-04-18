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
	
	FORCEINLINE float Byte_ToFloatNormalized( byte b )
	{
		const float Converter = 1.0f / 255.0f;
		return Clamp<float>( static_cast< float >( b * Converter ), 0.0f, 1.0f );
	}

	FORCEINLINE byte Float_ToByte( float f )
	{
		return static_cast< byte >( Clamp<float>( f, 0.0f, 1.0f ) * 255.0f );
	}

	FORCEINLINE float Lerp( float a, float b, float ratio )
	{
		return a + ( b - a ) * ratio;
	}


	/* SSE (Streaming SIMD Extensions) optimization
	*/
	namespace SSE
	{
		// single-precision floating point operators
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

		FORCEINLINE float Divide( float f, float divider )
		{
			__declspec( align(16) ) union V4SSE 
			{
				__m128 m;
				float vec[4];
			};
			V4SSE ret { _mm_div_ss( _mm_set_ss( f ), _mm_set_ss( divider ) ) };
			return ret.vec[0];
		}


		/* typedef XXM128
		*/
		typedef __m128	XXM128;

		// XXM128 operators
		//
		FORCEINLINE void XXM128_ToFloatArray_Aligned( float* dst, const XXM128& src )
		{
			_mm_store_ps( dst, src );
		}

		FORCEINLINE void XXM128_ToFloatArray_Unaligned( float* dst, const XXM128& src )
		{
			_mm_storeu_ps( dst, src );
		}

		FORCEINLINE XXM128 XXM128_Load( float x )
		{
			return _mm_set_ps( x, x, x, x );
		}

		FORCEINLINE XXM128 XXM128_Load( float x, float y, float z, float w )
		{
			return _mm_set_ps( w, z, y, x );	// little-endian
		}

		FORCEINLINE XXM128 XXM128_LoadAligned( const float* value )
		{
			return _mm_load_ps( value );
		}

		FORCEINLINE XXM128 XXM128_LoadUnaligned( const float* value )
		{
			return _mm_loadu_ps( value );
		}

		FORCEINLINE XXM128 XXM128_Negate( const XXM128& v )
		{
			return _mm_sub_ps( _mm_setzero_ps(), v );
		}

		FORCEINLINE XXM128 XXM128_Add( const XXM128& v1, const XXM128& v2 )
		{
			return _mm_add_ps( v1, v2 );
		}

		FORCEINLINE XXM128 XXM128_Subtract( const XXM128& v1, const XXM128& v2 )
		{
			return _mm_sub_ps( v1, v2 );
		}

		FORCEINLINE XXM128 XXM128_Multiply( const XXM128& v1, const XXM128& v2 )
		{
			return _mm_mul_ps( v1, v2 );
		}

		FORCEINLINE XXM128 XXM128_MultiplyAdd( const XXM128& v1, const XXM128& v2, const XXM128& adder )
		{
			return _mm_add_ps( _mm_mul_ps( v1, v2 ), adder );
		}

		FORCEINLINE XXM128 XXM128_Divide( const XXM128& v1, const XXM128& v2 )
		{
			return _mm_div_ps( v1, v2 );
		}

		FORCEINLINE XXM128 XXM128_Reciprocal( const XXM128& v )
		{
			return _mm_rcp_ps( v );
		}

		FORCEINLINE XXM128 XXM128_ReciprocalSqrt( const XXM128& v )
		{
			return _mm_rsqrt_ps( v );
		}

		FORCEINLINE XXM128 XXM128_Min( const XXM128& v1, const XXM128& v2 )
		{
			return _mm_min_ps( v1, v2 );
		}

		FORCEINLINE XXM128 XXM128_Max( const XXM128& v1, const XXM128& v2 )
		{
			return _mm_max_ps( v1, v2 );
		}
	};
};
