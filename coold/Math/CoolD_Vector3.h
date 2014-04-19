#pragma once

#include "CoolD_Math.h"

class Vector3
{
public:
    // constructor/destructor
    inline Vector3() {}
    inline Vector3( Dfloat _x, Dfloat _y, Dfloat _z ) :
        x(_x), y(_y), z(_z)
    {
    }
    inline ~Vector3() {}

    // copy operations
    Vector3(const Vector3& other);
	Vector3(const Dfloat* other);
    Vector3& operator=(const Vector3& other);

    // accessors
    inline Dfloat& operator[]( unsigned int i )          { return (&x)[i]; }
    inline Dfloat operator[]( unsigned int i ) const { return (&x)[i]; }

    Dfloat Length() const;
    Dfloat LengthSquared() const;

    friend Dfloat Distance( const Vector3& p0, const Vector3& p1 );
    friend Dfloat DistanceSquared( const Vector3& p0, const Vector3& p1 );

    // comparison
    Dbool operator==( const Vector3& other ) const;
    Dbool operator!=( const Vector3& other ) const;
    Dbool IsZero() const;
    Dbool IsUnit() const;

    // manipulators
    inline Dvoid Set( Dfloat _x, Dfloat _y, Dfloat _z );
    Dvoid Clean();       // sets near-zero elements to 0
	inline Dvoid Zero(); // sets all elements to 0
    Dvoid Normalize();   // sets to unit vector

    // operators

    // addition/subtraction
    Vector3 operator+( const Vector3& other ) const;
    friend Vector3& operator+=( Vector3& vector, const Vector3& other );
    Vector3 operator-( const Vector3& other ) const;
    friend Vector3& operator-=( Vector3& vector, const Vector3& other );

    Vector3 operator-() const;

    // scalar multiplication
    Vector3   operator*( Dfloat scalar );
    friend Vector3    operator*( Dfloat scalar, const Vector3& vector );
    Vector3&          operator*=( Dfloat scalar );
    Vector3   operator/( Dfloat scalar );
    Vector3&          operator/=( Dfloat scalar );

    // dot product/cross product
    Dfloat               Dot( const Vector3& vector ) const;
    friend Dfloat        Dot( const Vector3& vector1, const Vector3& vector2 );
    Vector3           Cross( const Vector3& vector ) const;
    friend Vector3    Cross( const Vector3& vector1, const Vector3& vector2 );

    // useful defaults
    static Vector3    xAxis;
    static Vector3    yAxis;
    static Vector3    zAxis;
    static Vector3    origin;

    // member variables
    Dfloat x, y, z;
};

inline Dvoid Vector3::Set( Dfloat _x, Dfloat _y, Dfloat _z )
{
    x = _x; y = _y; z = _z;
}  
inline Dvoid Vector3::Zero()
{
    x = y = z = 0.0f;
} 

