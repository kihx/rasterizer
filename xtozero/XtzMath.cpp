#include "stdafx.h"
#include "XtzMath.h"

namespace xtozero
{
	//------------------------------------------------------------------
	// class Matrix4
	//------------------------------------------------------------------

	Matrix4::Matrix4()
	{
		Identify();
	}

	Matrix4::Matrix4( const Matrix4& m )
	{
		Assign( m );
	}

	Matrix4::Matrix4( float _11, float _12, float _13, float _14,
		float _21, float _22, float _23, float _24,
		float _31, float _32, float _33, float _34,
		float _41, float _42, float _43, float _44 )
	{
		A[0][0] = _11; A[1][1] = _22; A[2][2] = _33; A[3][3] = _44;
		A[0][1] = _12; A[1][2] = _23; A[2][3] = _34; A[3][0] = _41;
		A[0][2] = _13; A[1][3] = _24; A[2][0] = _31; A[3][1] = _42;
		A[0][3] = _14; A[1][0] = _21; A[2][1] = _32; A[3][2] = _43;
	}

	Matrix4::~Matrix4()
	{
	}

	void Matrix4::Identify( void )
	{
		A[0][0] = A[1][1] = A[2][2] = A[3][3] = 1;
		A[0][1] = A[1][2] = A[2][3] = A[3][0] = 0;
		A[0][2] = A[1][3] = A[2][0] = A[3][1] = 0;
		A[0][3] = A[1][0] = A[2][1] = A[3][2] = 0;
	}

	void Matrix4::Assign( const Matrix4& m )
	{
		for ( int i = 0; i < 4; ++i )
		{
			for ( int j = 0; j < 4; ++j )
			{
				A[i][j] = m.A[i][j];
			}
		};
	}

	Matrix4& Matrix4::operator=(const Matrix4& m)
	{
		Assign( m );

		return *this;
	}

	float* Matrix4::operator[]( int row )
	{
		return A[row];
	}

	Matrix4& Matrix4::Transpose( void )
	{
		float f;

		for ( int i = 0; i < 4; ++i )
		{
			for ( int j = 0; j < 4; ++j )
			{
				// swap
				f = A[j][i];
				A[j][i] = A[i][j];
				A[i][j] = f;
			}
		}

		return *this;
	}

	Matrix4& Matrix4::Invert( void )
	{
		Matrix4 result;
		float* mat, *dest;

		mat = &(A[0][0]);
		dest = &(result.A[0][0]);

		float tmp[12]; /* temp array for pairs */
		float src[16]; /* array of transpose source matrix */
		float det; /* determinant */

		/* transpose matrix */
		for ( int i = 0; i < 4; ++i ) {
			src[i] = mat[i * 4];
			src[i + 4] = mat[i * 4 + 1];
			src[i + 8] = mat[i * 4 + 2];
			src[i + 12] = mat[i * 4 + 3];
		}

		/* calculate pairs for first 8 elements (cofactors) */
		tmp[0] = src[10] * src[15];
		tmp[1] = src[11] * src[14];
		tmp[2] = src[9] * src[15];
		tmp[3] = src[11] * src[13];
		tmp[4] = src[9] * src[14];
		tmp[5] = src[10] * src[13];
		tmp[6] = src[8] * src[15];
		tmp[7] = src[11] * src[12];
		tmp[8] = src[8] * src[14];
		tmp[9] = src[10] * src[12];
		tmp[10] = src[8] * src[13];
		tmp[11] = src[9] * src[12];

		/* calculate first 8 elements (cofactors) */
		dest[0] = tmp[0] * src[5] + tmp[3] * src[6] + tmp[4] * src[7];
		dest[0] -= tmp[1] * src[5] + tmp[2] * src[6] + tmp[5] * src[7];
		dest[1] = tmp[1] * src[4] + tmp[6] * src[6] + tmp[9] * src[7];
		dest[1] -= tmp[0] * src[4] + tmp[7] * src[6] + tmp[8] * src[7];
		dest[2] = tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7];
		dest[2] -= tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7];
		dest[3] = tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6];
		dest[3] -= tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6];
		dest[4] = tmp[1] * src[1] + tmp[2] * src[2] + tmp[5] * src[3];
		dest[4] -= tmp[0] * src[1] + tmp[3] * src[2] + tmp[4] * src[3];
		dest[5] = tmp[0] * src[0] + tmp[7] * src[2] + tmp[8] * src[3];
		dest[5] -= tmp[1] * src[0] + tmp[6] * src[2] + tmp[9] * src[3];
		dest[6] = tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3];
		dest[6] -= tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3];
		dest[7] = tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2];
		dest[7] -= tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2];

		/* calculate pairs for second 8 elements (cofactors) */
		tmp[0] = src[2] * src[7];
		tmp[1] = src[3] * src[6];
		tmp[2] = src[1] * src[7];
		tmp[3] = src[3] * src[5];
		tmp[4] = src[1] * src[6];
		tmp[5] = src[2] * src[5];
		tmp[6] = src[0] * src[7];
		tmp[7] = src[3] * src[4];
		tmp[8] = src[0] * src[6];
		tmp[9] = src[2] * src[4];
		tmp[10] = src[0] * src[5];
		tmp[11] = src[1] * src[4];

		/* calculate second 8 elements (cofactors) */
		dest[8] = tmp[0] * src[13] + tmp[3] * src[14] + tmp[4] * src[15];
		dest[8] -= tmp[1] * src[13] + tmp[2] * src[14] + tmp[5] * src[15];
		dest[9] = tmp[1] * src[12] + tmp[6] * src[14] + tmp[9] * src[15];
		dest[9] -= tmp[0] * src[12] + tmp[7] * src[14] + tmp[8] * src[15];
		dest[10] = tmp[2] * src[12] + tmp[7] * src[13] + tmp[10] * src[15];
		dest[10] -= tmp[3] * src[12] + tmp[6] * src[13] + tmp[11] * src[15];
		dest[11] = tmp[5] * src[12] + tmp[8] * src[13] + tmp[11] * src[14];
		dest[11] -= tmp[4] * src[12] + tmp[9] * src[13] + tmp[10] * src[14];
		dest[12] = tmp[2] * src[10] + tmp[5] * src[11] + tmp[1] * src[9];
		dest[12] -= tmp[4] * src[11] + tmp[0] * src[9] + tmp[3] * src[10];
		dest[13] = tmp[8] * src[11] + tmp[0] * src[8] + tmp[7] * src[10];
		dest[13] -= tmp[6] * src[10] + tmp[9] * src[11] + tmp[1] * src[8];
		dest[14] = tmp[6] * src[9] + tmp[11] * src[11] + tmp[3] * src[8];
		dest[14] -= tmp[10] * src[11] + tmp[2] * src[8] + tmp[7] * src[9];
		dest[15] = tmp[10] * src[10] + tmp[4] * src[8] + tmp[9] * src[9];
		dest[15] -= tmp[8] * src[9] + tmp[11] * src[10] + tmp[5] * src[8];

		/* calculate determinant */
		det = src[0] * dest[0] + src[1] * dest[1] + src[2] * dest[2] + src[3] * dest[3];

		/* calculate matrix inverse */
		det = 1 / det;
		for ( int j = 0; j < 16; ++j )
			dest[j] *= det;

		*this = result;

		return *this;
	}

	Matrix4& Matrix4::Translate( float x, float y, float z )
	{
		Identify();

		A[3][0] = x;
		A[3][1] = y;
		A[3][2] = z;

		return *this;
	}

	Matrix4& Matrix4::Scaling( float x, float y, float z )
	{
		Identify();

		A[0][0] = x;
		A[1][1] = y;
		A[2][2] = z;

		return *this;
	}


	Matrix4& Matrix4::RotateX( float angle )
	{
		float sinA = sin( angle );
		float cosA = cos( angle );

		Identify();
		A[1][1] = cosA; A[1][2] = sinA;
		A[2][1] = -sinA; A[2][2] = cosA;

		return *this;
	}

	Matrix4& Matrix4::RotateY( float angle )
	{
		float sinA = sin( angle );
		float cosA = cos( angle );

		Identify();
		A[0][0] = cosA; A[0][2] = -sinA;
		A[2][0] = sinA; A[2][2] = cosA;

		return *this;
	}

	Matrix4& Matrix4::RotateZ( float angle )
	{
		float sinA = sin( angle );
		float cosA = cos( angle );

		Identify();
		A[0][0] = cosA; A[0][1] = sinA;
		A[1][0] = -sinA; A[1][1] = cosA;

		return *this;
	}

	Matrix4& Matrix4::Rotate( const Vector3& axis, float angle )
	{
		float sinA = sin( angle );
		float cosA = cos( angle );
		float invCosA = 1.0f - cosA;

		Vector3 naxis = axis;
		naxis.Normalize();

		float x = naxis.X;
		float y = naxis.Y;
		float z = naxis.Z;

		float xSq = x * x;
		float ySq = y * y;
		float zSq = z * z;

		A[0][0] = (invCosA * xSq) + (cosA);
		A[1][0] = (invCosA * x * y) - (sinA * z);
		A[2][0] = (invCosA * x * z) + (sinA * y);
		A[3][0] = 0.0f;

		A[0][1] = (invCosA * x * y) + (sinA * z);
		A[1][1] = (invCosA * ySq) + (cosA);
		A[2][1] = (invCosA * y * z) - (sinA * x);
		A[3][1] = 0.0f;

		A[0][2] = (invCosA * x * z) - (sinA * y);
		A[1][2] = (invCosA * y * z) + (sinA * x);
		A[2][2] = (invCosA * zSq) + (cosA);
		A[3][2] = 0.0f;

		A[0][3] = 0.0f;
		A[1][3] = 0.0f;
		A[2][3] = 0.0f;
		A[3][3] = 1.0f;

		return *this;
	}

	Matrix4& Matrix4::LookAtLH( const Vector3& eye, const Vector3& at, const Vector3& up )
	{
		Vector3 zaxis = at - eye;
		zaxis.Normalize();

		Vector3 nup = up;
		nup.Normalize();

		Vector3 xaxis = zaxis.CrossProduct( nup );
		Vector3 yaxis = xaxis.CrossProduct( zaxis );

		A[0][0] = xaxis.X;	A[1][0] = xaxis.Y;	A[2][0] = xaxis.Z;	A[3][0] = 0.0f;
		A[0][1] = yaxis.X;	A[1][1] = yaxis.Y;	A[2][1] = yaxis.Z;	A[3][1] = 0.0f;
		A[0][2] = zaxis.X;	A[1][2] = zaxis.Y;	A[2][2] = zaxis.Z;	A[3][2] = 0.0f;
		A[0][3] = -xaxis.DotProduct( eye );		A[1][3] = -yaxis.DotProduct( eye );		A[2][3] = -zaxis.DotProduct( eye );		A[3][3] = 1.0f;

		return *this;
	}

	Matrix4& Matrix4::PerspectiveOffCenter( float l, float r, float b, float t, float zn, float zf )
	{
		float width = r - l;
		float height = t - b;
		float depth = zf - zn;

		// OpenGL
		A[0][0] = (2 * zn) / width;
		A[0][1] = 0.0f;
		A[0][2] = 0.0f;
		A[0][3] = 0.0f;

		A[1][0] = 0.0f;
		A[1][1] = (2 * zn) / height;
		A[1][2] = 0.0f;
		A[1][3] = 0.0f;

		A[2][0] = (r + l) / width;
		A[2][1] = (t + b) / height;
		A[2][2] = -(zf + zn) / depth;
		A[2][3] = -1.0f;

		A[3][0] = 0.0f;
		A[3][1] = 0.0f;
		A[3][2] = -(2 * zf * zn) / depth;
		A[3][3] = 0.0f;

		// DirectX
		/*A[0][0] = (2 * zn) / width;
		A[0][1] = 0.0f;
		A[0][2] = 0.0f;
		A[0][3] = 0.0f;

		A[1][0] = 0.0f;
		A[1][1] = (2 * zn) / height;
		A[1][2] = 0.0f;
		A[1][3] = 0.0f;

		A[2][0] = (r + l) / width;
		A[2][1] = (t + b) / height;
		A[2][2] = zf / depth;
		A[2][3] = 1.0f;

		A[3][0] = 0.0f;
		A[3][1] = 0.0f;
		A[3][2] = (zf * zn) / depth;
		A[3][3] = 0.0f;*/

		return *this;
	}

	Matrix4& Matrix4::PerspectiveLH( float fovY, float aspect, float zn, float zf )
	{
		float h = 1.0f / tanf( fovY * 0.5f );
		float w = h / aspect;
		float fn = zf - zn;

		M[0] = w;		M[1] = 0.0f;     M[2] = 0.0f;			M[3] = 0.0f;
		M[4] = 0.0f;    M[5] = h;		 M[6] = 0.0f;			M[7] = 0.0f;
		M[8] = 0.0f;    M[9] = 0.0f;     M[10] = zf / fn;		M[11] = 1.0f;
		M[12] = 0.0f;   M[13] = 0.0f;    M[14] = -zn * zf / fn; M[15] = 0.0f;

		return *this;
	}

	Matrix4& Matrix4::Ortho( float l, float r, float b, float t, float zn, float zf )
	{
		float width = r - l;
		float height = t - b;
		float depth = zf - zn;

		A[0][0] = 2.0f / width;
		A[0][1] = 0.0f;
		A[0][2] = 0.0f;
		A[0][3] = 0.0f;

		A[1][0] = 0.0f;
		A[1][1] = 2.0f / height;
		A[1][2] = 0.0f;
		A[1][3] = 0.0f;

		A[2][0] = 0.0f;
		A[2][1] = 0.0f;
		A[2][2] = -(2.0f) / depth;
		A[2][3] = 0.0f;

		A[3][0] = -(r + l) / width;
		A[3][1] = -(t + b) / height;
		A[3][2] = -(zf + zn) / depth;
		A[3][3] = 1.0f;

		return *this;
	}

	Matrix4& Matrix4::ViewPort( float l, float r, float b, float t, float zn, float zf )
	{
		float width = r - l;
		float height = t - b;

		A[0][0] = width / 2;
		A[0][1] = 0.0f;
		A[0][2] = 0.0f;
		A[0][3] = 0.0f;

		A[1][0] = 0.0f;
		A[1][1] = -height / 2;
		A[1][2] = 0.0f;
		A[1][3] = 0.0f;

		A[2][0] = 0.0f;
		A[2][1] = 0.0f;
		A[2][2] = zf - zn;
		A[2][3] = 0.0f;

		A[3][0] = l + width / 2;
		A[3][1] = t + height / 2;
		A[3][2] = zn;
		A[3][3] = 1.0f;

		return *this;
	}

	Matrix4 Matrix4::operator*(const Matrix4& rhs) const
	{
		Matrix4 result;
		float f;

		for ( int i = 0; i < 4; ++i )
		{
			for ( int j = 0; j < 4; ++j )
			{
				f = A[i][0] * rhs.A[0][j];
				f += A[i][1] * rhs.A[1][j];
				f += A[i][2] * rhs.A[2][j];
				f += A[i][3] * rhs.A[3][j];

				result.A[i][j] = f;
			}
		}

		return result;
	}

	Matrix4 Matrix4::operator*(float rhs) const
	{
		Matrix4 result;

		for ( int i = 0; i < 4; ++i )
		{
			for ( int j = 0; j < 4; ++j )
			{
				result.A[i][j] = result.A[i][j] * rhs;
			}
		}

		return result;
	}

	Matrix4& Matrix4::operator*=(const Matrix4&rhs)
	{
		Matrix4 result;
		float f;

		for ( int i = 0; i < 4; ++i )
		{
			for ( int j = 0; j < 4; ++j )
			{
				f = A[i][0] * rhs.A[0][j];
				f += A[i][1] * rhs.A[1][j];
				f += A[i][2] * rhs.A[2][j];
				f += A[i][3] * rhs.A[3][j];

				result.A[i][j] = f;
			}
		}
		*this = result;

		return *this;
	}

	Matrix4& Matrix4::operator*=(float rhs)
	{
		for ( int i = 0; i < 4; ++i )
		{
			for ( int j = 0; j < 4; ++j )
			{
				A[i][j] *= rhs;
			}
		}

		return *this;
	}

	Vector2::Vector2() : m_u( 0 ), m_v( 0 )
	{

	}

	Vector2::Vector2( float u, float v ) : m_u( u ), m_v( v )
	{

	}

	Vector2::~Vector2()
	{

	}

	const Vector2 Vector2::operator+(const Vector2& vector) const
	{
		return Vector2( m_u + vector.GetU(), m_v + vector.GetV() );
	}

	const Vector2 Vector2::operator+(const float scalar) const
	{
		return Vector2( m_u + scalar, m_v + scalar );
	}

	const Vector2 Vector2::operator-(const Vector2& vector) const
	{
		return Vector2( m_u - vector.GetU(), m_v - vector.GetV() );
	}

	const Vector2 Vector2::operator-(const float scalar) const
	{
		return Vector2( m_u - scalar, m_v - scalar );
	}

	const Vector2 Vector2::operator*(const Vector2& vector) const
	{
		return Vector2( m_u * vector.GetU(), m_v * vector.GetV() );
	}

	const Vector2 Vector2::operator*(const float scalar) const
	{
		return Vector2( m_u * scalar, m_v * scalar );
	}

	const Vector2 Vector2::operator/(const Vector2& vector) const
	{
		return Vector2( m_u / vector.GetU(), m_v / vector.GetV() );
	}

	const Vector2 Vector2::operator/(const float scalar) const
	{
		return Vector2( m_u / scalar, m_v / scalar );
	}

	Vector2& Vector2::operator+=(const Vector2& vector)
	{
		m_u += vector.GetU();
		m_v += vector.GetV();
		return *this;
	}

	Vector2& Vector2::operator+=(const float scalar)
	{
		m_u += scalar;
		m_v += scalar;
		return *this;
	}

	Vector2& Vector2::operator-=(const Vector2& vector)
	{
		m_u -= vector.GetU();
		m_v -= vector.GetV();
		return *this;
	}

	Vector2& Vector2::operator-=(const float scalar)
	{
		m_u -= scalar;
		m_v -= scalar;
		return *this;
	}

	Vector2& Vector2::operator*=(const Vector2& vector)
	{
		m_u *= vector.GetU();
		m_v *= vector.GetV();
		return *this;
	}

	Vector2& Vector2::operator*=(const float scalar)
	{
		m_u *= scalar;
		m_v *= scalar;
		return *this;
	}

	Vector2& Vector2::operator/=(const Vector2& vector)
	{
		m_u /= vector.GetU();
		m_v /= vector.GetV();;
		return *this;
	}

	Vector2& Vector2::operator/=(const float scalar)
	{
		m_u /= scalar; 
		m_v /= scalar;
		return *this;
	}

	Vector3::Vector3( void )
	{
		Set( 0, 0, 0 );
	}

	Vector3::Vector3( float x, float y, float z )
	{
		Set( x, y, z );
	}

	Vector3::~Vector3( void )
	{
	}


	void Vector3::Set( float x, float y, float z )
	{
		X = x;
		Y = y;
		Z = z;
	}

	float Vector3::Length( void )
	{
		return sqrt( X * X + Y * Y + Z * Z );
	}

	void Vector3::Normalize( void )
	{
		*this /= Length();
	}

	Vector3 Vector3::operator+(const Vector3& rhs) const
	{
		return Vector3( X + rhs.X,
			Y + rhs.Y,
			Z + rhs.Z );
	}

	Vector3 Vector3::operator+(float rhs) const
	{
		return Vector3( X + rhs,
			Y + rhs,
			Z + rhs );
	}

	Vector3 Vector3::operator-(const Vector3& rhs) const
	{
		return Vector3( X - rhs.X,
			Y - rhs.Y,
			Z - rhs.Z );
	}

	Vector3 Vector3::operator-(float rhs) const
	{
		return Vector3( X - rhs,
			Y - rhs,
			Z - rhs );
	}

	Vector3 Vector3::operator*(const Vector3& rhs) const
	{
		return Vector3( X * rhs.X,
			Y * rhs.Y,
			Z * rhs.Z );
	}

	Vector3 Vector3::operator*(float rhs) const
	{
		return Vector3( X * rhs,
			Y * rhs,
			Z * rhs );
	}

	Vector3 Vector3::operator/(const Vector3& rhs) const
	{
		return Vector3( X / rhs.X,
			Y / rhs.Y,
			Z / rhs.Z );
	}

	Vector3 Vector3::operator/(float rhs) const
	{
		return Vector3( X / rhs,
			Y / rhs,
			Z / rhs );
	}

	const Vector3& Vector3::operator+=(const Vector3 &v)
	{
		X += v.X;
		Y += v.Y;
		Z += v.Z;

		return *this;
	}

	const Vector3& Vector3::operator-=(const Vector3 &v)
	{
		X -= v.X;
		Y -= v.Y;
		Z -= v.Z;

		return *this;
	}

	const Vector3& Vector3::operator/=(float offset)
	{
		X /= offset;
		Y /= offset;
		Z /= offset;

		return *this;
	}

	const Vector3& Vector3::operator*=(float offset)
	{
		X *= offset;
		Y *= offset;
		Z *= offset;

		return *this;
	}

	const Vector3& Vector3::operator=(const Vector3& rhs)
	{
		X = rhs.X;
		Y = rhs.Y;
		Z = rhs.Z;

		return *this;
	}

	float Vector3::DotProduct( const Vector3& v3 )
	{
		return X * v3.X + Y * v3.Y + Z * v3.Z;
	}


	float Vector3::operator^(const Vector3& v3)
	{
		return DotProduct( v3 );
	}

	Vector3 Vector3::CrossProduct( const Vector3& v3 )
	{
		return Vector3( Y * v3.Z - Z * v3.Y,
			Z * v3.X - X * v3.Z,
			X * v3.Y - Y * v3.X );
	}

	Vector3 Vector3::operator%(const Vector3& v3)
	{
		return CrossProduct( v3 );
	}

	Vector3& Vector3::Transform( Matrix4& mat )
	{
		Vector3 tmp;
		tmp.X = mat.A[0][0] * X + mat.A[1][0] * Y + mat.A[2][0] * Z + mat.A[3][0];
		tmp.Y = mat.A[0][1] * X + mat.A[1][1] * Y + mat.A[2][1] * Z + mat.A[3][1];
		tmp.Z = mat.A[0][2] * X + mat.A[1][2] * Y + mat.A[2][2] * Z + mat.A[3][2];
		float w = mat.A[0][3] * X + mat.A[1][3] * Y + mat.A[2][3] * Z + mat.A[3][3];

		tmp.X /= w;
		tmp.Y /= w;
		tmp.Z /= w;

		*this = tmp;

		return *this;
	}

	Vector4::Vector4( void )
	{
		Set( 0.0f, 0.0f, 0.0f, 1.0f );
	}

	Vector4::Vector4( float x, float y, float z, float w )
	{
		Set( x, y, z, w );
	}

	Vector4::~Vector4( void )
	{
	}

	void Vector4::Set( float x, float y, float z, float w )
	{
		X = x;
		Y = y;
		Z = z;
		W = w;
	}

	float Vector4::Length( void )
	{
		return sqrt( (X * X) + (Y * Y) + (Z * Z) );
	}

	void Vector4::Normalize( void )
	{
		float length = Length();

		X /= length;
		Y /= length;
		Z /= length;
	}

	Vector4 Vector4::operator+(const Vector4& rhs) const
	{
		return Vector4( X + rhs.X,
			Y + rhs.Y,
			Z + rhs.Z );
	}

	Vector4 Vector4::operator+(float rhs) const
	{
		return Vector4( X + rhs,
			Y + rhs,
			Z + rhs );
	}

	Vector4 Vector4::operator-(const Vector4& rhs) const
	{
		return Vector4( X - rhs.X,
			Y - rhs.Y,
			Z - rhs.Z );
	}

	Vector4 Vector4::operator-(float rhs) const
	{
		return Vector4( X - rhs,
			Y - rhs,
			Z - rhs );
	}

	Vector4 Vector4::operator*(const Vector4& rhs) const
	{
		return Vector4( X * rhs.X,
			Y * rhs.Y,
			Z * rhs.Z );
	}

	Vector4 Vector4::operator*(float rhs) const
	{
		return Vector4( X * rhs,
			Y * rhs,
			Z * rhs );
	}

	Vector4 Vector4::operator/(const Vector4& rhs) const
	{
		return Vector4( X / rhs.X,
			Y / rhs.Y,
			Z / rhs.Z,
			W / rhs.W );
	}

	Vector4 Vector4::operator/(float rhs) const
	{
		return Vector4( X / rhs,
			Y / rhs,
			Z / rhs,
			W / rhs );
	}

	const Vector4& Vector4::operator+=(const Vector4 &v)
	{
		X += v.X;
		Y += v.Y;
		Z += v.Z;

		return *this;
	}

	const Vector4& Vector4::operator-=(const Vector4 &v)
	{
		X -= v.X;
		Y -= v.Y;
		Z -= v.Z;

		return *this;
	}

	const Vector4& Vector4::operator/=(float offset)
	{
		X /= offset;
		Y /= offset;
		Z /= offset;
		W /= offset;

		return *this;
	}

	const Vector4& Vector4::operator*=(float offset)
	{
		X *= offset;
		Y *= offset;
		Z *= offset;

		return *this;
	}

	const Vector4& Vector4::operator=(const Vector4& rhs)
	{
		X = rhs.X;
		Y = rhs.Y;
		Z = rhs.Z;

		return *this;
	}

	float Vector4::DotProduct( const Vector4& v3 )
	{
		return X * v3.X + Y * v3.Y + Z * v3.Z;
	}

	float Vector4::operator^(const Vector4& v3)
	{
		return DotProduct( v3 );
	}

	Vector4 Vector4::CrossProduct( const Vector4& v3 )
	{
		return Vector4( Y * v3.Z - Z * v3.Y,
			Z * v3.X - X * v3.Z,
			X * v3.Y - Y * v3.X );
	}

	Vector4 Vector4::operator%(const Vector4& v3)
	{
		return CrossProduct( v3 );
	}

	Vector4& Vector4::Transform( const Matrix4& mat )
	{
		Vector4 tmp;
		tmp.X = mat.A[0][0] * X + mat.A[1][0] * Y + mat.A[2][0] * Z + mat.A[3][0];
		tmp.Y = mat.A[0][1] * X + mat.A[1][1] * Y + mat.A[2][1] * Z + mat.A[3][1];
		tmp.Z = mat.A[0][2] * X + mat.A[1][2] * Y + mat.A[2][2] * Z + mat.A[3][2];
		tmp.W = mat.A[0][3] * X + mat.A[1][3] * Y + mat.A[2][3] * Z + mat.A[3][3];

		X = tmp.X;
		Y = tmp.Y;
		Z = tmp.Z;
		W = tmp.W;

		return *this;
	}
}