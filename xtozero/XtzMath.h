#ifndef _XTZMATH_H_
#define _XTZMATH_H_

#define DEG2RAD(a)		a * 0.01745329252f
#define RAD2DEG(a)		a * 57.29577951f

#include <vector>
#include <math.h>

namespace xtozero
{
	class Vector3;
	class Vector4;

	class Matrix4
	{
		// Fields
		//
	public:
		union
		{
			struct MatrixElement
			{
				float        A11, A12, A13, A14;
				float        A21, A22, A23, A24;
				float        A31, A32, A33, A34;
				float        A41, A42, A43, A44;
			};
			float A[4][4];
			float M[16];
		};

		// Methods
		//
	public:
		// ctor / dtor
		Matrix4( void );

		Matrix4( const Matrix4& m );

		Matrix4( float _11, float _12, float _13, float _14,
			float _21, float _22, float _23, float _24,
			float _31, float _32, float _33, float _34,
			float _41, float _42, float _43, float _44 );

		~Matrix4( void );

		void Identify( void );

		/* static Matrix4 GetIdentity( void )
		{
		return Matrix4( );
		}*/

		void Assign( const Matrix4& m );

		Matrix4& Transpose( void );

		Matrix4& Invert( void );

		Matrix4& Translate( float x, float y, float z );

		Matrix4& Scaling( float x, float y, float z );

		Matrix4& RotateX( float angle );

		Matrix4& RotateY( float angle );

		Matrix4& RotateZ( float angle );

		Matrix4& Rotate( const Vector3& axis, float angle );

		Matrix4& LookAtLH( const Vector3& eye, const Vector3& at, const Vector3& up );

		Matrix4& PerspectiveOffCenter( float l, float r, float b, float t, float zn, float zf );

		Matrix4& PerspectiveLH( float fovY, float aspect, float zn, float zf );

		Matrix4& Ortho( float l, float r, float b, float t, float zn, float zf );

		Matrix4& ViewPort( float l, float r, float b, float t, float zn, float zf );

		Matrix4& operator=(const Matrix4& m);

		float* operator[]( int row );

		Matrix4 operator*(const Matrix4& rhs) const;

		Matrix4 operator*(float rhs) const;

		// Vector3 operator*(const Vector3 &v) const;

		Matrix4& operator*=(const Matrix4& rhs);

		Matrix4& operator*=(float rhs);
	};

	class Vector2
	{
	private:
		float m_u;
		float m_v;

	public:
		Vector2();
		Vector2( float u, float v );
		~Vector2();

		const float GetU() const
		{
			return m_u;
		}
		const float GetV() const
		{
			return m_v;
		}
		const Vector2 operator+(const Vector2& vector2) const;
		const Vector2 operator+(const float scalar) const;

		const Vector2 operator-(const Vector2& vector2) const;
		const Vector2 operator-(const float scalar) const;

		const Vector2 operator*(const Vector2& vector2) const;
		const Vector2 operator*(const float scalar) const;

		const Vector2 operator/(const Vector2& vector2) const;
		const Vector2 operator/(const float scalar) const;

		Vector2& operator+=(const Vector2& vector2);
		Vector2& operator+=(const float scalar);

		Vector2& operator-=(const Vector2& vector2);
		Vector2& operator-=(const float scalar);

		Vector2& operator*=(const Vector2& vector2);
		Vector2& operator*=(const float scalar);

		Vector2& operator/=(const Vector2& vector2);
		Vector2& operator/=(const float scalar);
	};

	class Vector3
	{
		// Fields
		//
	public:
		float X, Y, Z;

		// Methods
		//
	public:
		// ctor / dtor		
		Vector3( void );

		Vector3( float x, float y, float z );

		~Vector3( void );

		void Set( float x, float y, float z );

		float Length( void );

		void Normalize( void );

		Vector3 operator+(const Vector3& rhs) const;

		Vector3 operator+(float rhs) const;

		Vector3 operator-(const Vector3& rhs) const;

		Vector3 operator-(float rhs) const;

		Vector3 operator*(const Vector3& rhs) const;

		Vector3 operator*(float rhs) const;

		Vector3 operator/(const Vector3& rhs) const;

		Vector3 operator/(float rhs) const;

		// assignment operators
		const Vector3& operator+=(const Vector3 &v);

		const Vector3& operator-=(const Vector3 &v);

		const Vector3& operator/=(float offset);

		const Vector3& operator*=(float offset);

		const Vector3& operator=(const Vector3& rhs);

		float DotProduct( const Vector3& v3 );

		float operator^(const Vector3& v3);

		Vector3 CrossProduct( const Vector3& v3 );

		Vector3 operator%(const Vector3& v3);

		Vector3& Transform( Matrix4& mat );
	};

	class Vector4
	{
		// Fields
		//
	public:
		float X, Y, Z, W;

		// Methods
		//
	public:
		// ctor / dtor		
		Vector4( void );

		Vector4( float x, float y, float z, float w = 1.0f );

		~Vector4( void );

		void Set( float x, float y, float z, float w = 1.0f );

		float Length( void );

		void Normalize( void );

		Vector4 operator+(const Vector4& rhs) const;

		Vector4 operator+(float rhs) const;

		Vector4 operator-(const Vector4& rhs) const;

		Vector4 operator-(float rhs) const;

		Vector4 operator*(const Vector4& rhs) const;

		Vector4 operator*(float rhs) const;

		Vector4 operator/(const Vector4& rhs) const;

		Vector4 operator/(float rhs) const;

		const Vector4& operator+=(const Vector4 &v);

		const Vector4& operator-=(const Vector4 &v);

		const Vector4& operator/=(float offset);

		const Vector4& operator*=(float offset);

		const Vector4& operator=(const Vector4& rhs);

		float DotProduct( const Vector4& v3 );

		float operator^(const Vector4& v3);

		Vector4 CrossProduct( const Vector4& v3 );

		Vector4 operator%(const Vector4& v3);

		Vector4& Transform( const Matrix4& mat );
	};

	template<typename T>
	T Lerp( T start, T end, float ratio )
	{
		return start + (end - start) * ratio;
	}
}

#endif