#pragma once

#include "CoolD_Math.h"

class Vector2
{
public:
    // constructor/destructor
    inline Vector2() {}
    inline Vector2( Dfloat _x, Dfloat _y ) :
        x(_x), y(_y)
    {
    }
    inline ~Vector2() {}
    
    // accessors
    inline Dfloat& operator[]( unsigned int i )          { return (&x)[i]; }
    inline Dfloat operator[]( unsigned int i ) const { return (&x)[i]; }

    Dfloat Length() const;
    Dfloat LengthSquared() const;

    // comparison
    Dbool operator==( const Vector2& other ) const;
    Dbool operator!=( const Vector2& other ) const;
    Dbool IsZero() const;

    // manipulators
    inline Dvoid Set( Dfloat _x, Dfloat _y );
    Dvoid Clean();       // sets near-zero elements to 0
    inline Dvoid Zero(); // sets all elements to 0
    Dvoid Normalize();   // sets to unit vector

    // operators
    Vector2 operator-() const;

    // addition/subtraction
    Vector2 operator+( const Vector2& other ) const;
    Vector2& operator+=( const Vector2& other );
    Vector2 operator-( const Vector2& other ) const;
    Vector2& operator-=( const Vector2& other );

    // scalar multiplication
    Vector2   operator*( Dfloat scalar );
    friend Vector2    operator*( Dfloat scalar, const Vector2& vector );
    Vector2&          operator*=( Dfloat scalar );
    Vector2   operator/( Dfloat scalar );
    Vector2&          operator/=( Dfloat scalar );

    // dot product
    Dfloat               Dot( const Vector2& vector ) const;
    friend Dfloat        Dot( const Vector2& vector1, const Vector2& vector2 );

    // perpendicular and cross product equivalent
    Vector2 Perpendicular() const { return Vector2(-y, x); } 
    Dfloat               PerpDot( const Vector2& vector ) const; 
    friend Dfloat        PerpDot( const Vector2& vector1, const Vector2& vector2 );

    // useful defaults
    static Vector2    xAxis;
    static Vector2    yAxis;
    static Vector2    origin;
    static Vector2    xy;
        
    // member variables
    Dfloat x, y;

protected:

private:
};

inline Dvoid Vector2::Set( Dfloat _x, Dfloat _y )
{
    x = _x; y = _y;
}  

inline Dvoid Vector2::Zero()
{
    x = y = 0.0f;
} 