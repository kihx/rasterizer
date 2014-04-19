#pragma once

#include "CoolD_Math.h"
#include "CoolD_Vector3.h"

class Vector4
{	
public:
    // constructor/destructor
    inline Vector4() {}
    inline Vector4( Dfloat _x, Dfloat _y, Dfloat _z, Dfloat _w ) :
        x(_x), y(_y), z(_z), w(_w)
    {
    }
	inline Vector4(const Vector3& v3, Dfloat _w) :
		x(v3.x), y(v3.y), z(v3.z), w(_w)
	{
	}
    inline ~Vector4() {}

    // copy operations
    Vector4(const Vector4& other);
    Vector4& operator=(const Vector4& other);

    // accessors
    inline Dfloat& operator[]( unsigned int i )         { return (&x)[i]; }
    inline Dfloat operator[]( unsigned int i ) const    { return (&x)[i]; }

    Dfloat Length() const;
    Dfloat LengthSquared() const;

    // comparison
    Dbool operator==( const Vector4& other ) const;
    Dbool operator!=( const Vector4& other ) const;
    Dbool IsZero() const;
    Dbool IsUnit() const;

    // manipulators
    inline Dvoid Set( Dfloat _x, Dfloat _y, Dfloat _z, Dfloat _w );
    Dvoid Clean();       // sets near-zero elements to 0
    inline Dvoid Zero(); // sets all elements to 0
    Dvoid Normalize();   // sets to unit vector

    // operators

    // addition/subtraction
    Vector4 operator+( const Vector4& other ) const;
    Vector4& operator+=( const Vector4& other );
    Vector4 operator-( const Vector4& other ) const;
    Vector4& operator-=( const Vector4& other );

    // scalar multiplication
    Vector4    operator*( Dfloat scalar );
    friend Vector4    operator*( Dfloat scalar, const Vector4& vector );
    Vector4&          operator*=( Dfloat scalar );
    Vector4			  operator/(Dfloat scalar) const;
    Vector4&          operator/=( Dfloat scalar );
	
    // dot product
    Dfloat              Dot( const Vector4& vector ) const;
    friend Dfloat       Dot( const Vector4& vector1, const Vector4& vector2 );

    // useful defaults
    static Vector4    xAxis;
    static Vector4    yAxis;
    static Vector4    zAxis;
    static Vector4    wAxis;
    static Vector4    origin;
        
    // member variables
    Dfloat x, y, z, w;

public:
	enum W_MODE { W_IGNORE, W_DIVIDE };
	friend Vector3 Vec4ToVec3( const Vector4& vector, W_MODE value );
};

inline Dvoid Vector4::Set( Dfloat _x, Dfloat _y, Dfloat _z, Dfloat _w )
{
    x = _x; y = _y; z = _z; w = _w;
}

inline Dvoid Vector4::Zero()
{
    x = y = z = w = 0.0f;
} 
