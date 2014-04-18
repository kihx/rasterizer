#pragma once

#include "base.h"
#include "mathlib.h"


namespace kih
{
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

		void NormalizeInPlace()
		{
			*this /= Length();
		}

		float DotProduct( const Vector3& rhs )
		{
			return X * rhs.X + Y * rhs.Y + Z * rhs.Z;
		}

		Vector3 CrossProduct( const Vector3& rhs )
		{
			return Vector3( Y * rhs.Z - Z * rhs.Y, Z * rhs.X - X * rhs.Z, X * rhs.Y - Y * rhs.X );
		}

		Vector3 operator+( const Vector3& rhs ) const
		{
			return Vector3( X + rhs.X, Y + rhs.Y, Z + rhs.Z );
		}

		Vector3 operator+( float rhs ) const
		{
			return Vector3( X + rhs, Y + rhs, Z + rhs );
		}

		Vector3 operator-( const Vector3& rhs ) const
		{
			return Vector3( X - rhs.X, Y - rhs.Y, Z - rhs.Z );
		}

		Vector3 operator-( float rhs ) const
		{
			return Vector3( X - rhs, Y - rhs, Z - rhs );
		}

		Vector3 operator*( const Vector3& rhs ) const
		{
			return Vector3( X * rhs.X, Y * rhs.Y, Z * rhs.Z );
		}

		Vector3 operator*( float rhs ) const
		{
			return Vector3( X * rhs, Y * rhs, Z * rhs );
		}

		Vector3 operator/( const Vector3& rhs ) const
		{
			return Vector3( X / rhs.X, Y / rhs.Y, Z / rhs.Z );
		}

		Vector3 operator/( float rhs ) const
		{
			return Vector3( X / rhs, Y / rhs, Z / rhs );
		}

		Vector3& operator+=( const Vector3 &rhs )
		{
			X += rhs.X;
			Y += rhs.Y;
			Z += rhs.Z;
			return *this;
		}

		Vector3& operator-=( const Vector3 &rhs )
		{
			X -= rhs.X;
			Y -= rhs.Y;
			Z -= rhs.Z;
			return *this;
		}

		Vector3& operator/=( float value )
		{
			X /= value;
			Y /= value;
			Z /= value;
			return *this;
		}

		Vector3& operator*=( float value )
		{
			X *= value;
			Y *= value;
			Z *= value;
			return *this;
		}

		Vector3& operator=( const Vector3& rhs )
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

		bool operator==( const Vector3& rhs ) const
		{
			return X == rhs.X && Y == rhs.Y && Z == rhs.Z;
		}

		bool operator!=( const Vector3& rhs ) const
		{
			return X != rhs.X || Y != rhs.Y || Z != rhs.Z;
		}

		float operator[]( int index ) const
		{
			Assert( index >= 0 && index < 3 );
			return Value[index];
		}

		float& operator[]( int index )
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

		Vector4& operator+=( const Vector4 &rhs )
		{
			X += rhs.X;
			Y += rhs.Y;
			Z += rhs.Z;
			W += rhs.W;
			return *this;
		}

		Vector4& operator-=( const Vector4 &rhs )
		{
			X -= rhs.X;
			Y -= rhs.Y;
			Z -= rhs.Z;
			W -= rhs.W;
			return *this;
		}

		Vector4& operator/=( float value )
		{
			X /= value;
			Y /= value;
			Z /= value;
			W /= value;
			return *this;
		}

		Vector4& operator*=( float value )
		{
			X *= value;
			Y *= value;
			Z *= value;
			W *= value;
			return *this;
		}

		bool operator==( const Vector4& rhs ) const
		{
			return X == rhs.X && Y == rhs.Y && Z == rhs.Z && W == rhs.W;
		}

		bool operator!=( const Vector4& rhs ) const
		{
			return X != rhs.X || Y != rhs.Y || Z != rhs.Z || W != rhs.W;
		}

		float operator[]( int index ) const
		{
			Assert( index >= 0 && index < 4 );
			return Value[index];
		}

		float& operator[]( int index )
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

	
	inline Color32 Vector4_ToColor32( const Vector4& src )
	{
		return Color32( Float_ToByte( src.X ), Float_ToByte( src.Y ), Float_ToByte( src.Z ), Float_ToByte( src.W ) );
	}
}


using kih::Vector3;
using kih::Vector4;

using kih::Color32;
using kih::Color128;
