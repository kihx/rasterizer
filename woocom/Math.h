#ifndef _MATH_3D_H_
#define _MATH_3D_H_


//------------------------------------------------------------------
// Include Directives
//------------------------------------------------------------------
#include <vector>


//------------------------------------------------------------------
// Using Directives
//------------------------------------------------------------------
#define DEG2RAD(a)		a * 0.01745329252f
#define RAD2DEG(a)		a * 57.29577951f

class Vector3;
class Matrix4;


// Defines a vector 3D.
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
	Vector3(void);

	Vector3(float x, float y, float z);

	~Vector3(void);

	void Set(float x, float y, float z);

	inline float Length(void);

	inline void Normalize(void);

	// casting
	//operator float*();

	//operator const float*() const;

	//// unary operators
	//Vector3 operator+() const;

	//Vector3 operator-() const;

	// binary operators
	inline Vector3 operator+(const Vector3& rhs) const;

	inline Vector3 operator+(float rhs) const;

	inline Vector3 operator-(const Vector3& rhs) const;

	inline Vector3 operator-(float rhs) const;

	inline Vector3 operator*(const Vector3& rhs) const;

	inline Vector3 operator*(const Matrix4& rhs) const;

	inline Vector3 operator*(float rhs) const;

	inline Vector3 operator/(const Vector3& rhs) const;

	inline Vector3 operator/(float rhs) const;
	
	// assignment operators
	inline const Vector3& operator+=(const Vector3 &v);

	inline const Vector3& operator-=(const Vector3 &v);

	inline const Vector3& operator/=(float offset);

	inline const Vector3& operator*=(float offset);

	inline const Vector3& operator=(const Vector3& rhs);

	inline float DotProduct(const Vector3& v3);

	inline float operator^(const Vector3& v3);

	inline Vector3 CrossProduct(const Vector3& v3);

	inline Vector3 operator%(const Vector3& v3);

	// equality operators
	inline bool operator==(const Vector3&) const;

	inline bool operator!=(const Vector3&) const;

	Vector3& Transform(Matrix4& mat);
};

inline Vector3::Vector3(void)
{
	Set(0, 0, 0);
}

inline Vector3::Vector3(float x, float y, float z)
{
	Set(x, y, z);
}

inline Vector3::~Vector3(void)
{
}


inline void Vector3::Set(float x, float y, float z)
{
	X = x;
	Y = y;
	Z = z;
}

inline float Vector3::Length(void)
{
	return sqrt(X * X + Y * Y + Z * Z);
}

inline void Vector3::Normalize(void)
{
	*this /= Length();
}

inline Vector3 Vector3::operator+(const Vector3& rhs) const
{
	return Vector3(X + rhs.X,
		Y + rhs.Y,
		Z + rhs.Z);
}

inline Vector3 Vector3::operator+(float rhs) const
{
	return Vector3(X + rhs,
		Y + rhs,
		Z + rhs);
}

inline Vector3 Vector3::operator-(const Vector3& rhs) const
{
	return Vector3(X - rhs.X,
		Y - rhs.Y,
		Z - rhs.Z);
}

inline Vector3 Vector3::operator-(float rhs) const
{
	return Vector3(X - rhs,
		Y - rhs,
		Z - rhs);
}

inline Vector3 Vector3::operator*(const Vector3& rhs) const
{
	return Vector3(X * rhs.X,
		Y * rhs.Y,
		Z * rhs.Z);
}

inline Vector3 Vector3::operator*(float rhs) const
{
	return Vector3(X * rhs,
		Y * rhs,
		Z * rhs);
}

inline Vector3 Vector3::operator/(const Vector3& rhs) const
{
	return Vector3(X / rhs.X,
		Y / rhs.Y,
		Z / rhs.Z);
}

inline Vector3 Vector3::operator/(float rhs) const
{
	return Vector3(X / rhs,
		Y / rhs,
		Z / rhs);
}

inline const Vector3& Vector3::operator+=(const Vector3 &v)
{
	X += v.X;
	Y += v.Y;
	Z += v.Z;

	return *this;
}

inline const Vector3& Vector3::operator-=(const Vector3 &v)
{
	X -= v.X;
	Y -= v.Y;
	Z -= v.Z;

	return *this;
}

inline const Vector3& Vector3::operator/=(float offset)
{
	X /= offset;
	Y /= offset;
	Z /= offset;

	return *this;
}

inline const Vector3& Vector3::operator*=(float offset)
{
	X *= offset;
	Y *= offset;
	Z *= offset;

	return *this;
}

inline const Vector3& Vector3::operator=(const Vector3& rhs)
{
	X = rhs.X;
	Y = rhs.Y;
	Z = rhs.Z;

	return *this;
}

inline float Vector3::DotProduct(const Vector3& v3)
{
	return X * v3.X + Y * v3.Y + Z * v3.Z;
}


inline float Vector3::operator^(const Vector3& v3)
{
	return DotProduct(v3);
}

inline Vector3 Vector3::CrossProduct(const Vector3& v3)
{
	return Vector3(Y * v3.Z - Z * v3.Y,
		Z * v3.X - X * v3.Z,
		X * v3.Y - Y * v3.X);
}

inline Vector3 Vector3::operator%(const Vector3& v3)
{
	return CrossProduct(v3);
}


// Describes a matrix 4 by 4.
class Matrix4
{
	// Fields
	//
public:
	union
	{
		struct DATA
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
	Matrix4(void);

	inline Matrix4(const Matrix4& m);

	Matrix4(float _11, float _12, float _13, float _14,
		float _21, float _22, float _23, float _24,
		float _31, float _32, float _33, float _34,
		float _41, float _42, float _43, float _44);

	~Matrix4(void);

	inline void Identify(void);

	inline static Matrix4 GetIdentity(void)
	{
		return Matrix4();
	}

	inline void Assign(const Matrix4& m);

	Matrix4& Transpose(void);

	Matrix4& Invert(void);

	Matrix4& Translate(float x, float y, float z);

	Matrix4& Scaling(float x, float y, float z);

	Matrix4& RotateX(float angle);

	Matrix4& RotateY(float angle);

	Matrix4& RotateZ(float angle);

	Matrix4& Rotate(const Vector3& axis, float angle);

	Matrix4& LookAtLH(const Vector3& eye, const Vector3& at, const Vector3& up);

	Matrix4& PerspectiveOffCenter(float l, float r, float b, float t, float zn, float zf);

	Matrix4& PerspectiveLH(float fovY, float aspect, float zn, float zf);

	Matrix4& Ortho(float l, float r, float b, float t, float zn, float zf);

	Matrix4& ViewPort(float l, float r, float b, float t, float zn, float zf);

	//// access grants
	//float& operator()(int Row, int Col);
	//float  operator()(int Row, int Col) const;

	//// casting operators
	//operator float*();
	//operator const float*() const;

	//// unary operators
	//Matrix4 operator+() const;
	//Matrix4 operator-() const;

	//// equality operators
	//bool operator==(const Matrix4&) const;
	//bool operator!=(const Matrix4&) const;

	inline Matrix4& operator=(const Matrix4& m);

	inline float* operator[](int row);

	Matrix4 operator*(const Matrix4& rhs) const;

	Matrix4 operator*(float rhs) const;

	//inline Vector3 operator*(const Vector3 &v) const;

	inline Matrix4& operator*=(const Matrix4& rhs);

	inline Matrix4& operator*=(float rhs);
};


inline Matrix4::Matrix4()
{
	Identify();
}

inline Matrix4::Matrix4(const Matrix4& m)
{
	Assign(m);
}

inline Matrix4::Matrix4(float _11, float _12, float _13, float _14,
	float _21, float _22, float _23, float _24,
	float _31, float _32, float _33, float _34,
	float _41, float _42, float _43, float _44)
{
	A[0][0] = _11; A[1][1] = _22; A[2][2] = _33; A[3][3] = _44;
	A[0][1] = _12; A[1][2] = _23; A[2][3] = _34; A[3][0] = _41;
	A[0][2] = _13; A[1][3] = _24; A[2][0] = _31; A[3][1] = _42;
	A[0][3] = _14; A[1][0] = _21; A[2][1] = _32; A[3][2] = _43;
}

inline Matrix4::~Matrix4()
{
}

inline void Matrix4::Identify(void)
{
	A[0][0] = A[1][1] = A[2][2] = A[3][3] = 1;
	A[0][1] = A[1][2] = A[2][3] = A[3][0] = 0;
	A[0][2] = A[1][3] = A[2][0] = A[3][1] = 0;
	A[0][3] = A[1][0] = A[2][1] = A[3][2] = 0;
}

inline void Matrix4::Assign(const Matrix4& m)
{
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			A[i][j] = m.A[i][j];
		}
	};
}

inline Matrix4& Matrix4::operator=(const Matrix4& m)
{
	Assign(m);

	return *this;
}

inline float* Matrix4::operator[](int row)
{
	return A[row];
}

inline Vector3 Vector3::operator*(const Matrix4& rhs) const
{
	return Vector3(
		X * rhs.A[0][0] + Y * rhs.A[1][0] + Z * rhs.A[2][0] + rhs.A[3][0],
		X * rhs.A[0][1] + Y * rhs.A[1][1] + Z * rhs.A[2][1] + rhs.A[3][1],
		X * rhs.A[0][2] + Y * rhs.A[1][2] + Z * rhs.A[2][2] + rhs.A[3][2]);
}


#endif

