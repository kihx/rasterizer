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
			union V4SSE 
			{
				__m128 m;
				float vec[4];
			};
			V4SSE ret { _mm_div_ss( _mm_set_ss( f ), _mm_set_ss( divider ) ) };
			return ret.vec[0];
		}


		/* typedef Vector4SSE
		*/
		typedef __m128	Vector4SSE;

		// Vector4SSE operators
		//
		FORCEINLINE void Vector4SSE_ToFloatArray_Aligned( float* dst, const Vector4SSE& src )
		{
			_mm_store_ps( dst, src );
		}

		FORCEINLINE void Vector4SSE_ToFloatArray_Unaligned( float* dst, const Vector4SSE& src )
		{
			_mm_storeu_ps( dst, src );
		}

		FORCEINLINE Vector4SSE MakeVector4SSE( float x, float y, float z, float w )
		{
			return _mm_set_ps( w, z, y, x );	// little-endian?
		}

		FORCEINLINE Vector4SSE MakeVector4SSE_Aligned( const float* value )
		{
			return _mm_load_ps( value );
		}

		FORCEINLINE Vector4SSE MakeVector4SSE_Unaligned( const float* value )
		{
			return _mm_loadu_ps( value );
		}

		FORCEINLINE Vector4SSE Vector4SSE_Negate( const Vector4SSE& v )
		{
			return _mm_sub_ps( _mm_setzero_ps(), v );
		}

		FORCEINLINE Vector4SSE Vector4SSE_Add( const Vector4SSE& v1, const Vector4SSE& v2 )
		{
			return _mm_add_ps( v1, v2 );
		}

		FORCEINLINE Vector4SSE Vector4SSE_Subtract( const Vector4SSE& v1, const Vector4SSE& v2 )
		{
			return _mm_sub_ps( v1, v2 );
		}

		FORCEINLINE Vector4SSE Vector4SSE_Multiply( const Vector4SSE& v1, const Vector4SSE& v2 )
		{
			return _mm_mul_ps( v1, v2 );
		}

		FORCEINLINE Vector4SSE Vector4SSE_MultiplyAdd( const Vector4SSE& v1, const Vector4SSE& v2, const Vector4SSE& adder )
		{
			return _mm_add_ps( _mm_mul_ps( v1, v2 ), adder );
		}

		FORCEINLINE Vector4SSE Vector4SSE_Divide( const Vector4SSE& v1, const Vector4SSE& v2 )
		{
			return _mm_div_ps( v1, v2 );
		}

		FORCEINLINE Vector4SSE Vector4SSE_Reciprocal( const Vector4SSE& v )
		{
			return _mm_rcp_ps( v );
		}

		FORCEINLINE Vector4SSE Vector4SSE_ReciprocalSqrt( const Vector4SSE& v )
		{
			return _mm_rsqrt_ps( v );
		}

		FORCEINLINE Vector4SSE Vector4SSE_Min( const Vector4SSE& v1, const Vector4SSE& v2 )
		{
			return _mm_min_ps( v1, v2 );
		}

		FORCEINLINE Vector4SSE Vector4SSE_Max( const Vector4SSE& v1, const Vector4SSE& v2 )
		{
			return _mm_max_ps( v1, v2 );
		}
	};
};
