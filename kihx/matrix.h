#pragma once

#include "base.h"
#include "vector.h"


namespace kih
{
	struct Matrix4
	{
		union
		{
			struct
			{
				float A11, A12, A13, A14;
				float A21, A22, A23, A24;
				float A31, A32, A33, A34;
				float A41, A42, A43, A44;
			};
			float A[4][4];
			float Value[16];
		};

		Matrix4() :
			A11( 1.0f ), A12( 0.0f ), A13( 0.0f ), A14( 0.0f ),
			A21( 0.0f ), A22( 1.0f ), A23( 0.0f ), A24( 0.0f ),
			A31( 0.0f ), A32( 0.0f ), A33( 1.0f ), A34( 0.0f ),
			A41( 0.0f ), A42( 0.0f ), A43( 0.0f ), A44( 1.0f )
		{
		}

		Matrix4( const float* mat ) :
			A11( mat[0] ), A12( mat[1] ), A13( mat[2] ), A14( mat[3] ),
			A21( mat[4] ), A22( mat[5] ), A23( mat[6] ), A24( mat[7] ),
			A31( mat[8] ), A32( mat[9] ), A33( mat[10] ), A34( mat[11] ),
			A41( mat[12] ), A42( mat[13] ), A43( mat[14] ), A44( mat[15] )
		{
		}

		Matrix4( const Matrix4& rhs ) = default;

		Matrix4( float _11, float _12, float _13, float _14,
			float _21, float _22, float _23, float _24,
			float _31, float _32, float _33, float _34,
			float _41, float _42, float _43, float _44 ) :
			A11( _11 ), A12( _12 ), A13( _13 ), A14( _14 ),
			A21( _21 ), A22( _22 ), A23( _23 ), A24( _24 ),
			A31( _31 ), A32( _32 ), A33( _33 ), A34( _34 ),
			A41( _41 ), A42( _42 ), A43( _43 ), A44( _44 )
		{
		}

		void Identify()
		{
			A[0][0] = A[1][1] = A[2][2] = A[3][3] = 1.0f;
			A[0][1] = A[1][2] = A[2][3] = A[3][0] = 0.0f;
			A[0][2] = A[1][3] = A[2][0] = A[3][1] = 0.0f;
			A[0][3] = A[1][0] = A[2][1] = A[3][2] = 0.0f;
		}

		const Matrix4& Transpose();

		const Matrix4& Invert();

		const Matrix4& Translate( float x, float y, float z );

		const Matrix4& Scaling( float x, float y, float z );

		const Matrix4& RotateX( float angle );

		const Matrix4& RotateY( float angle );

		const Matrix4& RotateZ( float angle );

		const Matrix4& Rotate( const Vector3& axis, float angle );

		const Matrix4& LookAtLH( const Vector3& eye, const Vector3& at, const Vector3& up );

		const Matrix4& PerspectiveOffCenter( float l, float r, float b, float t, float zn, float zf );

		const Matrix4& PerspectiveLH( float fovY, float aspect, float zn, float zf );

		const Matrix4& Ortho( float l, float r, float b, float t, float zn, float zf );

		const Matrix4& ViewPort( float l, float r, float b, float t, float zn, float zf );

		Matrix4 operator*( const Matrix4& rhs ) const
		{
			Matrix4 ret;
			float f;

			for ( int i = 0; i < 4; ++i )
			{
				for ( int j = 0; j < 4; ++j )
				{
					f = A[i][0] * rhs.A[0][j];
					f += A[i][1] * rhs.A[1][j];
					f += A[i][2] * rhs.A[2][j];
					f += A[i][3] * rhs.A[3][j];
					ret.A[i][j] = f;
				}
			}

			return ret;
		}

		Matrix4& operator=( const Matrix4& rhs )
		{
			if ( this == &rhs )
			{
				return *this;
			}

			for ( int i = 0; i < 16; ++i )
			{
				Value[i] = rhs.Value[i];
			};
			return *this;
		}

		const float* operator[]( int row )
		{
			Assert( row >= 0 && row < 4 );
			return A[row];
		}
	};


	/*
	*/
	void Vector3_Transform( const Vector3& vec, const Matrix4& mat, Vector4& result );
	void Vector3_TransformSSE( const Vector3& vec, const Matrix4& mat, Vector4& result );

	void Vector4_Transform( const Vector4& vec, const Matrix4& mat, Vector4& result );
	void Vector4_TransformSSE( const Vector4& vec, const Matrix4& mat, Vector4& result );
}

using kih::Matrix4;
