#pragma once

#include "CoolD_Math.h"

class Vector3;
class Matrix33;

class Quat
{
public:
    // constructor/destructor
    inline Quat() : w(1.0f), x(0.0f), y(0.0f), z(0.0f)
    {}
    inline Quat( Dfloat _w, Dfloat _x, Dfloat _y, Dfloat _z ) :
        w(_w), x(_x), y(_y), z(_z)
    {
    }
    Quat( const Vector3& axis, Dfloat angle );
    Quat( const Vector3& from, const Vector3& to );
    explicit Quat( const Vector3& vector );
    explicit Quat( const Matrix33& rotation );
    inline ~Quat() {}

    // copy operations
    Quat(const Quat& other);
    Quat& operator=(const Quat& other);

    // accessors
    inline Dfloat& operator[]( unsigned int i )         { return (&x)[i]; }
    inline Dfloat operator[]( unsigned int i ) const    { return (&x)[i]; }

    Dfloat Magnitude() const;
    Dfloat Norm() const;

    // comparison
    Dbool operator==( const Quat& other ) const;
    Dbool operator!=( const Quat& other ) const;
    Dbool IsZero() const;
    Dbool IsUnit() const;
    Dbool IsIdentity() const;

    // manipulators
    inline Dvoid Set( Dfloat _w, Dfloat _x, Dfloat _y, Dfloat _z );
    Dvoid Set( const Vector3& axis, Dfloat angle );
    Dvoid Set( const Vector3& from, const Vector3& to );
    Dvoid Set( const Matrix33& rotation );
    Dvoid Set( Dfloat zRotation, Dfloat yRotation, Dfloat xRotation ); 

    Dvoid GetAxisAngle( Vector3& axis, Dfloat& angle );

    Dvoid Clean();       // sets near-zero elements to 0
    inline Dvoid Zero(); // sets all elements to 0
    Dvoid Normalize();   // sets to unit quaternion
    inline Dvoid Identity();    // sets to identity quaternion

    // complex conjugate
    friend Quat Conjugate( const Quat& quat );
    const Quat& Conjugate();

    // invert quaternion
    friend Quat Inverse( const Quat& quat );
    const Quat& Inverse();

    // operators

    // addition/subtraction
    Quat operator+( const Quat& other ) const;
    Quat& operator+=( const Quat& other );
    Quat operator-( const Quat& other ) const;
    Quat& operator-=( const Quat& other );

    Quat operator-() const;

    // scalar multiplication
    friend Quat    operator*( Dfloat scalar, const Quat& vector );
    Quat&          operator*=( Dfloat scalar );

    // quaternion multiplication
    Quat operator*( const Quat& other ) const;
    Quat& operator*=( const Quat& other );

    // dot product
    Dfloat              Dot( const Quat& vector ) const;
    friend Dfloat       Dot( const Quat& vector1, const Quat& vector2 );

    // vector rotation
    Vector3   Rotate( const Vector3& vector ) const;

    // interpolation
    friend Dvoid Lerp( Quat& result, const Quat& start, const Quat& end, Dfloat t );
    friend Dvoid Slerp( Quat& result, const Quat& start, const Quat& end, Dfloat t );
    friend Dvoid ApproxSlerp( Quat& result, const Quat& start, const Quat& end, Dfloat t );

    // useful defaults
    static Quat   zero;
    static Quat   identity;
        
    // member variables
    Dfloat w, x, y, z;      

protected:

private:
};

inline Dvoid Quat::Set( Dfloat _w, Dfloat _x, Dfloat _y, Dfloat _z )
{
    w = _w; x = _x; y = _y; z = _z;
}  

inline Dvoid Quat::Zero()
{
    x = y = z = w = 0.0f;
}  

inline Dvoid 
Quat::Identity()
{
    x = y = z = 0.0f;
    w = 1.0f;
}   