#pragma once

#include "base.h"
#include "vector.h"

#include <intrin.h>


namespace kih
{
	namespace SSE
	{
		/* typedef xxm128
		*/
		typedef __m128	xxm128;
		typedef __m128i	xxm128i;
	};


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
		return ( lhs < rhs ) ? lhs : rhs;
	}

	template<class T>
	FORCEINLINE const T& Max( const T& lhs, const T& rhs )
	{
		static_assert( std::is_integral<T>::value || std::is_floating_point<T>::value, "base type must be integral or floating point" );
		return ( lhs > rhs ) ? lhs : rhs;
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

	FORCEINLINE Vector3 Lerp( const Vector3& a, const Vector3& b, float ratio )
	{
		return a + ( b - a ) * ratio;
	}
	
	FORCEINLINE Color32 Vector4_ToColor32( const Vector4& src )
	{
		return Color32( Float_ToByte( src.X ), Float_ToByte( src.Y ), Float_ToByte( src.Z ), Float_ToByte( src.W ) );
	}

	// Test whether if the specified triangle is toward back in View or ViewProj space.
	bool IsBackFace( const Vector4& v0, const Vector4& v1, const Vector4& v2 );

	template<class T>
	FORCEINLINE T ComputeBarycentric( const Vector2Base<T>& p, const Vector2Base<T>& v0, const Vector2Base<T>& v1 )
	{
		return ( v1.X - v0.X ) * ( p.Y - v0.Y ) - ( v1.Y - v0.Y ) * ( p.X - v0.X );
	}

	template<class T>
	FORCEINLINE bool IsPointInTriangle( const Vector2Base<T>& p, const Vector2Base<T>& v0, const Vector2Base<T>& v1, const Vector2Base<T>& v2 )
	{
		T o0 = ComputeBarycentric( p, v0, v1 );
		T o1 = ComputeBarycentric( p, v1, v2 );
		T o2 = ComputeBarycentric( p, v2, v0 );
		return o0 >= 0 && o1 >= 0 && o2 >= 0;
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

		FORCEINLINE xxm128 Round( const xxm128& v )
		{
			return _mm_round_ps( v, _MM_FROUND_NINT );
		}

		FORCEINLINE float Divide( float f, float divider )
		{
			_CRT_ALIGN(16) union V4SSE			
			{
				__m128 m;
				float vec[4];
			};
			V4SSE ret { _mm_div_ss( _mm_set_ss( f ), _mm_set_ss( divider ) ) };
			return ret.vec[0];
		}


		/* typedef xxm128
		*/
		#define SWIZZLE_MASK(fp3, fp2, fp1, fp0)	(((fp3) << 6) | ((fp2) << 4) | ((fp1) << 2) | ((fp0)))
		

		// xxm128 operators
		//
		FORCEINLINE xxm128i ReinterpretCast_xxm128i( const xxm128& v )
		{
			return _mm_cvttps_epi32( v );
		}

		FORCEINLINE xxm128 ReinterpretCast_xxm128( const xxm128i& v )
		{
			return _mm_cvtepi32_ps( v );
		}

		FORCEINLINE int ReinterpretCast_int( const xxm128& v )
		{
			return _mm_cvtt_ss2si( v );
		}

		FORCEINLINE xxm128 Load( float x )
		{
			return _mm_set_ps1( x );
		}

		FORCEINLINE xxm128i Load( int x )
		{
			return _mm_set1_epi32( x );
		}		

		FORCEINLINE xxm128 Load( float x, float y, float z, float w )
		{
			return _mm_set_ps( w, z, y, x );	// little-endian
		}

		FORCEINLINE xxm128i Load( int x, int y, int z, int w )
		{
			return _mm_set_epi32( w, z, y, x );	// little-endian
		}		

		FORCEINLINE xxm128 LoadReverse( float x, float y, float z, float w )
		{
			return _mm_set_ps( x, y, z, w );	// little-endian
		}

		FORCEINLINE xxm128i LoadReverse( int x, int y, int z, int w )
		{
			return _mm_setr_epi32( x, y, z, w );	// little-endian
		}

		FORCEINLINE xxm128 LoadAligned( const float* value )
		{
			return _mm_load_ps( value );
		}

		FORCEINLINE xxm128 LoadUnaligned( const float* value )
		{
			return _mm_loadu_ps( value );
		}

		FORCEINLINE void Store( float& dst, const xxm128& src )
		{
			_mm_store_ss( &dst, src );
		}

		FORCEINLINE void StoreAligned( float* dst, const xxm128& src )
		{
			_mm_store_ps( dst, src );
		}

		FORCEINLINE void StoreUnaligned( float* dst, const xxm128& src )
		{
			_mm_storeu_ps( dst, src );
		}

		FORCEINLINE xxm128 Negate( const xxm128& v )
		{
			return _mm_sub_ps( _mm_setzero_ps(), v );
		}

		FORCEINLINE xxm128 Add( const xxm128& v1, const xxm128& v2 )
		{
			return _mm_add_ps( v1, v2 );
		}

		FORCEINLINE xxm128i Add( const xxm128i& v1, const xxm128i& v2 )
		{
			return _mm_add_epi32( v1, v2 );
		}		

		FORCEINLINE xxm128 Subtract( const xxm128& v1, const xxm128& v2 )
		{
			return _mm_sub_ps( v1, v2 );
		}

		FORCEINLINE xxm128 Multiply( const xxm128& v1, const xxm128& v2 )
		{
			return _mm_mul_ps( v1, v2 );
		}

		FORCEINLINE xxm128 MultiplyAdd( const xxm128& v1, const xxm128& v2, const xxm128& adder )
		{
			return _mm_add_ps( _mm_mul_ps( v1, v2 ), adder );
		}

		FORCEINLINE xxm128 Divide( const xxm128& v1, const xxm128& v2 )
		{
			return _mm_div_ps( v1, v2 );
		}

		FORCEINLINE xxm128 Reciprocal( const xxm128& v )
		{
			return _mm_rcp_ps( v );
		}

		FORCEINLINE xxm128 Sqrt( const xxm128& v )
		{
			return _mm_sqrt_ps( v );
		}

		FORCEINLINE xxm128 ReciprocalSqrt( const xxm128& v )
		{
			return _mm_rsqrt_ps( v );
		}

		FORCEINLINE xxm128 Min( const xxm128& v1, const xxm128& v2 )
		{
			return _mm_min_ps( v1, v2 );
		}

		FORCEINLINE xxm128i Min( const xxm128i& v1, const xxm128i& v2 )
		{
			return _mm_min_epi32( v1, v2 );
		}

		FORCEINLINE xxm128 Max( const xxm128& v1, const xxm128& v2 )
		{
			return _mm_max_ps( v1, v2 );
		}

		FORCEINLINE xxm128i Max( const xxm128i& v1, const xxm128i& v2 )
		{
			return _mm_max_epi32( v1, v2 );
		}

		// avoiding "error C2057: expected constant expression"
		#define Swizzle( /*const xxm128&*/ v, /*unsigned int*/ mask ) \
			_mm_shuffle_ps( v, v, mask )

		FORCEINLINE xxm128 DotProduct( const xxm128& v1, const xxm128& v2 )
		{
			// dot product 4 values (1111) 
			// and store them at all (1111) 
			// 11111111 = 0xFF 
			const int Mask = 0xFF;
			return _mm_dp_ps( v1, v2, Mask );
		}

		FORCEINLINE xxm128 Length( const xxm128& v )
		{
			xxm128 dot = DotProduct( v, v );
			return Sqrt( dot );
		}

		FORCEINLINE xxm128 ReciprocalLength( const xxm128& v )
		{
			xxm128 dot = DotProduct( v, v );
			return ReciprocalSqrt( dot );
		}

		FORCEINLINE xxm128 Normalize( const xxm128& v )
		{
			xxm128 recipLen = ReciprocalLength( v );
			return Multiply( v, recipLen );
		}

		FORCEINLINE xxm128 CrossProduct( const xxm128& v1, const xxm128& v2 )
		{
			xxm128 v1YZXW = Swizzle( v1, SWIZZLE_MASK( 1, 2, 0, 3 ) );
			xxm128 v1ZXYW = Swizzle( v1, SWIZZLE_MASK( 2, 0, 1, 3 ) );			
			
			xxm128 v2YZXW = Swizzle( v2, SWIZZLE_MASK( 1, 2, 0, 3 ) );
			xxm128 v2ZXYW = Swizzle( v2, SWIZZLE_MASK( 2, 0, 1, 3 ) );

			return Subtract( Multiply( v1YZXW, v2ZXYW ), Multiply( v1ZXYW, v2YZXW ) );
		}

		FORCEINLINE xxm128 Lerp( const xxm128& a, const xxm128& b, const xxm128& ratio )
		{	
			return Add( a, Multiply( Subtract( b, a ), ratio ) );
		}

		bool IsBackFace( const Vector4& v0, const Vector4& v1, const Vector4& v2 );
		bool IsBackFace( const xxm128& v0, const xxm128& v1, const xxm128& v2 );


		// Comparison functions
		FORCEINLINE xxm128 Equal( const xxm128& v1, const xxm128& v2 )
		{
			return _mm_cmpeq_ps( v1, v2 );
		}		

		FORCEINLINE xxm128 Less( const xxm128& v1, const xxm128& v2 )
		{
			return _mm_cmplt_ps( v1, v2 );
		}

		FORCEINLINE xxm128i Less( const xxm128i& v1, const xxm128i& v2 )
		{
			return _mm_cmplt_epi32( v1, v2 );
		}		 

		FORCEINLINE xxm128 LessEqual( const xxm128& v1, const xxm128& v2 )
		{
			return _mm_cmple_ss( v1, v2 );
		}

		FORCEINLINE xxm128i Greater( const xxm128i& v1, const xxm128i& v2 )
		{
			return _mm_cmpgt_epi32( v1, v2 );
		}

		FORCEINLINE xxm128i Or( const xxm128i& v1, const xxm128i& v2 )
		{
			return _mm_or_si128( v1, v2 );
		}

		FORCEINLINE xxm128i And( const xxm128i& v1, const xxm128i& v2 )
		{
			return _mm_and_si128( v1, v2 );
		}

		FORCEINLINE xxm128i AndNot( const xxm128i& v1, const xxm128i& v2 )
		{
			return _mm_andnot_si128( v1, v2 );
		}
		
		FORCEINLINE bool IsAllZero( const xxm128i& v )
		{
			// _mm_movemask_epi8( _mm_cmpeq_epi8( v, _mm_setzero_si128() ) ) == 0xFFFF;	// SSE2
			return _mm_test_all_zeros( v, v ) != 0;		// SSE4
		}

		// Vector constants
		const static xxm128 xxm128_Zero = _mm_setzero_ps();
		const static xxm128 xxm128_One = SSE::Load( 1.0f );

		const static xxm128i xxm128i_Zero = _mm_setzero_si128();
	};
};

using kih::SSE::xxm128;
using kih::SSE::xxm128i;


