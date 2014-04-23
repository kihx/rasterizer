#pragma once

#include "base.h"


namespace kih
{
	/* struct Vector2Base
	*/
	template<class T>
	struct Vector2Base
	{
		static_assert( std::is_integral<T>::value || std::is_floating_point<T>::value, "invalid type of Vector2Base" );

		union
		{
			struct
			{
				T X;
				T Y;
			};

			T Value[2];
		};

		Vector2Base() :
			X( T() ),
			Y( T() )
		{
		}

		Vector2Base( T x, T y ) :
			X( x ),
			Y( y )
		{
		}

		Vector2Base( const T* rhs ) :
			X( rhs[0] ),
			Y( rhs[1] )
		{
		}

		Vector2Base( const Vector2Base& rhs ) :
			X( rhs.X ),
			Y( rhs.Y )
		{
		}

		FORCEINLINE Vector2Base& operator=( const Vector2Base& rhs )
		{
			if ( this == &rhs )
			{
				return *this;
			}

			X = rhs.X;
			Y = rhs.Y;
			return *this;
		}
	};
	typedef Vector2Base<float> Vector2;
	typedef Vector2Base<int> Vector2i;


	/* struct Vector3
	*/
	struct Vector3
	{
		union
		{
			struct
			{
				float X;
				float Y;
				float Z;
			};

			float Value[3];
		};

		Vector3() :
			X( 0.0f ),
			Y( 0.0f ),
			Z( 0.0f )
		{
		}
		
		Vector3( float x, float y, float z ) :
			X( x ),
			Y( y ),
			Z( z )
		{
		}
		
		Vector3( const float* rhs ) :
			X( rhs[0] ),
			Y( rhs[1] ),
			Z( rhs[2] )
		{
		}

		Vector3( const Vector3& rhs ) :
			X( rhs.X ),
			Y( rhs.Y ),
			Z( rhs.Z )
		{
		}

		float Length()
		{
			return sqrt( DotProduct( *this ) );
		}

		Vector3 Normalize()
		{
			Vector3 normalized = *this;
			normalized /= normalized.Length();
			return normalized;
		}

		FORCEINLINE void NormalizeInPlace()
		{
			*this /= Length();
		}

		FORCEINLINE float DotProduct( const Vector3& rhs ) const
		{
			return X * rhs.X + Y * rhs.Y + Z * rhs.Z;
		}

		FORCEINLINE Vector3 CrossProduct( const Vector3& rhs ) const
		{
			return Vector3( Y * rhs.Z - Z * rhs.Y, Z * rhs.X - X * rhs.Z, X * rhs.Y - Y * rhs.X );
		}

		FORCEINLINE Vector3 operator+( const Vector3& rhs ) const
		{
			return Vector3( X + rhs.X, Y + rhs.Y, Z + rhs.Z );
		}

		FORCEINLINE Vector3 operator+( float rhs ) const
		{
			return Vector3( X + rhs, Y + rhs, Z + rhs );
		}

		FORCEINLINE Vector3 operator-( const Vector3& rhs ) const
		{
			return Vector3( X - rhs.X, Y - rhs.Y, Z - rhs.Z );
		}

		FORCEINLINE Vector3 operator-( float rhs ) const
		{
			return Vector3( X - rhs, Y - rhs, Z - rhs );
		}

		FORCEINLINE Vector3 operator*( const Vector3& rhs ) const
		{
			return Vector3( X * rhs.X, Y * rhs.Y, Z * rhs.Z );
		}

		FORCEINLINE Vector3 operator*( float rhs ) const
		{
			return Vector3( X * rhs, Y * rhs, Z * rhs );
		}

		FORCEINLINE Vector3 operator/( const Vector3& rhs ) const
		{
			return Vector3( X / rhs.X, Y / rhs.Y, Z / rhs.Z );
		}

		FORCEINLINE Vector3 operator/( float rhs ) const
		{
			return Vector3( X / rhs, Y / rhs, Z / rhs );
		}

		FORCEINLINE Vector3& operator+=( const Vector3 &rhs )
		{
			X += rhs.X;
			Y += rhs.Y;
			Z += rhs.Z;
			return *this;
		}

		FORCEINLINE Vector3& operator-=( const Vector3 &rhs )
		{
			X -= rhs.X;
			Y -= rhs.Y;
			Z -= rhs.Z;
			return *this;
		}

		FORCEINLINE Vector3& operator/=( float value )
		{
			X /= value;
			Y /= value;
			Z /= value;
			return *this;
		}

		FORCEINLINE Vector3& operator*=( float value )
		{
			X *= value;
			Y *= value;
			Z *= value;
			return *this;
		}

		FORCEINLINE Vector3& operator=( const Vector3& rhs )
		{
			if ( this == &rhs )
			{
				return *this;
			}

			X = rhs.X;
			Y = rhs.Y;
			Z = rhs.Z;
			return *this;
		}

		FORCEINLINE bool operator==( const Vector3& rhs ) const
		{
			return X == rhs.X && Y == rhs.Y && Z == rhs.Z;
		}

		FORCEINLINE bool operator!=( const Vector3& rhs ) const
		{
			return X != rhs.X || Y != rhs.Y || Z != rhs.Z;
		}

		FORCEINLINE float operator[]( int index ) const
		{
			Assert( index >= 0 && index < 3 );
			return Value[index];
		}

		FORCEINLINE float& operator[]( int index )
		{
			Assert( index >= 0 && index < 3 );
			return Value[index];
		}
	};


	/* struct Vector4
	*/
	_CRT_ALIGN( 16 ) struct Vector4
	{
		union
		{
			struct
			{
				float X;
				float Y;
				float Z;
				float W;
			};

			float Value[4];
		};

		Vector4() :
			X( 0.0f ),
			Y( 0.0f ),
			Z( 0.0f ),
			W( 1.0f )
		{
		}

		Vector4( float x, float y, float z, float w = 1.0f ) :
			X( x ),
			Y( y ),
			Z( z ),
			W( w )
		{
		}

		Vector4( const float* rhs ) :
			X( rhs[0] ),
			Y( rhs[1] ),
			Z( rhs[2] ),
			W( rhs[3] )
		{
		}

		Vector4( const Vector3& rhs ) :
			X( rhs.X ),
			Y( rhs.Y ),
			Z( rhs.Z ),
			W( 1.0f )
		{
		}

		Vector4( const Vector4& rhs ) = default;

		Vector4& operator=( const Vector4& rhs ) = default;

		FORCEINLINE Vector4 operator+( const Vector4& rhs ) const
		{
			return Vector4( X + rhs.X, Y + rhs.Y, Z + rhs.Z, W + rhs.W );
		}

		FORCEINLINE Vector4 operator+( float rhs ) const
		{
			return Vector4( X + rhs, Y + rhs, Z + rhs, W + rhs );
		}

		FORCEINLINE Vector4 operator-( const Vector4& rhs ) const
		{
			return Vector4( X - rhs.X, Y - rhs.Y, Z - rhs.Z );
		}

		FORCEINLINE Vector4 operator-( float rhs ) const
		{
			return Vector4( X - rhs, Y - rhs, Z - rhs );
		}

		FORCEINLINE Vector4& operator+=( const Vector4 &rhs )
		{
			X += rhs.X;
			Y += rhs.Y;
			Z += rhs.Z;
			W += rhs.W;
			return *this;
		}

		FORCEINLINE Vector4& operator-=( const Vector4 &rhs )
		{
			X -= rhs.X;
			Y -= rhs.Y;
			Z -= rhs.Z;
			W -= rhs.W;
			return *this;
		}

		FORCEINLINE Vector4& operator/=( float value )
		{
			X /= value;
			Y /= value;
			Z /= value;
			W /= value;
			return *this;
		}

		FORCEINLINE Vector4& operator*=( float value )
		{
			X *= value;
			Y *= value;
			Z *= value;
			W *= value;
			return *this;
		}

		FORCEINLINE bool operator==( const Vector4& rhs ) const
		{
			return X == rhs.X && Y == rhs.Y && Z == rhs.Z && W == rhs.W;
		}

		FORCEINLINE bool operator!=( const Vector4& rhs ) const
		{
			return X != rhs.X || Y != rhs.Y || Z != rhs.Z || W != rhs.W;
		}

		FORCEINLINE float operator[]( int index ) const
		{
			Assert( index >= 0 && index < 4 );
			return Value[index];
		}

		FORCEINLINE float& operator[]( int index )
		{
			Assert( index >= 0 && index < 4 );
			return Value[index];
		}
	};


	/* struct Color4
	*/
	template<class T>
	struct Color4
	{
		static_assert( std::is_integral<T>::value || std::is_floating_point<T>::value, "base type must be integral or floating point" );

		union
		{
			struct
			{
				T R;
				T G;
				T B;
				T A;
			};

			T Value[4];
		};

		Color4() :
			R( T() ),
			G( T() ),
			B( T() ),
			A( T() )
		{
		}

		Color4( T r, T g, T b, T a ) :
			R( r ),
			G( g ),
			B( b ),
			A( a )
		{
		}

		Color4( const T color[4] ) :
			R( color[0] ),
			G( color[1] ),
			B( color[2] ),
			A( color[3] )
		{
		}

		Color4( const Color4& c ) = default;
	};
	

	typedef Color4<byte> Color32;
	typedef Color4<float> Color128;
}


using kih::Vector3;
using kih::Vector4;

using kih::Color32;
using kih::Color128;
