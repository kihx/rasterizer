#pragma once


#include "base.h"


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

		Vector3& operator/=( float offset )
		{
			X /= offset;
			Y /= offset;
			Z /= offset;
			return *this;
		}

		Vector3& operator*=( float offset )
		{
			X *= offset;
			Y *= offset;
			Z *= offset;
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
	};


	/* struct Vector4
	*/
	struct Vector4
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

		Vector4( float x, float y, float z, float w ) :
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

		Vector4( const Vector4& rhs ) = default;

		Vector4& operator=( const Vector4& rhs ) = default;
	};
}

using kih::Vector3;
using kih::Vector4;
