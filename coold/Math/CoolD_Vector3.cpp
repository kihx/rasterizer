#include "CoolD_Vector3.h"

Vector3 Vector3::xAxis( 1.0f, 0.0f, 0.0f );
Vector3 Vector3::yAxis( 0.0f, 1.0f, 0.0f );
Vector3 Vector3::zAxis( 0.0f, 0.0f, 1.0f );
Vector3 Vector3::origin( 0.0f, 0.0f, 0.0f );

Vector3::Vector3(const Vector3& other) : 
    x( other.x ),
    y( other.y ),
    z( other.z )
{

}   

Vector3::Vector3(const Dfloat* other) :
	x(other[ 0 ]),
	y(other[ 1 ]),
	z(other[ 2 ])
{

}

Vector3& Vector3::operator=(const Vector3& other)
{
    // if same object
    if ( this == &other )
        return *this;
        
    x = other.x;
    y = other.y;
    z = other.z;
    
    return *this;

}   

Dfloat Vector3::Length() const
{
    return Sqrt( x*x + y*y + z*z );

}  

Dfloat Vector3::LengthSquared() const
{
    return (x*x + y*y + z*z);

}   

Dfloat Distance( const Vector3& p0, const Vector3& p1 )
{
    Dfloat x = p0.x - p1.x;
    Dfloat y = p0.y - p1.y;
    Dfloat z = p0.z - p1.z;

    return ::Sqrt( x*x + y*y + z*z );

}   

Dfloat DistanceSquared( const Vector3& p0, const Vector3& p1 )
{
    Dfloat x = p0.x - p1.x;
    Dfloat y = p0.y - p1.y;
    Dfloat z = p0.z - p1.z;

    return ( x*x + y*y + z*z );

}   

Dbool Vector3::operator==( const Vector3& other ) const
{
    if ( ::AreEqual( other.x, x )
        && ::AreEqual( other.y, y )
        && ::AreEqual( other.z, z ) )
        return true;

    return false;   
}   

Dbool Vector3::operator!=( const Vector3& other ) const
{
    if ( ::AreEqual( other.x, x )
        && ::AreEqual( other.y, y )
        && ::AreEqual( other.z, z ) )
        return false;

    return true;
}   

Dbool Vector3::IsZero() const
{
    return ::IsZero(x*x + y*y + z*z);

}  

Dbool Vector3::IsUnit() const
{
    return ::IsZero(1.0f - x*x - y*y - z*z);

}  

Dvoid Vector3::Clean()
{
    if ( ::IsZero( x ) )
        x = 0.0f;
    if ( ::IsZero( y ) )
        y = 0.0f;
    if ( ::IsZero( z ) )
        z = 0.0f;

}   

Dvoid Vector3::Normalize()
{
    Dfloat lengthsq = x*x + y*y + z*z;

    if ( ::IsZero( lengthsq ) )
    {
        Zero();
    }
    else
    {
        Dfloat factor = InvSqrt( lengthsq );
        x *= factor;
        y *= factor;
        z *= factor;
    }

}  

Vector3 Vector3::operator+( const Vector3& other ) const
{
    return Vector3( x + other.x, y + other.y, z + other.z );

}  

Vector3& operator+=( Vector3& self, const Vector3& other )
{
    self.x += other.x;
    self.y += other.y;
    self.z += other.z;

    return self;

}   

Vector3 Vector3::operator-( const Vector3& other ) const
{
    return Vector3( x - other.x, y - other.y, z - other.z );

}   

Vector3& operator-=( Vector3& self, const Vector3& other )
{
    self.x -= other.x;
    self.y -= other.y;
    self.z -= other.z;

    return self;

}   

Vector3 Vector3::operator-() const
{
    return Vector3(-x, -y, -z);
}   

Vector3 Vector3::operator*( Dfloat scalar )
{
    return Vector3( scalar*x, scalar*y, scalar*z );

}   

Vector3 operator*( Dfloat scalar, const Vector3& vector )
{
    return Vector3( scalar*vector.x, scalar*vector.y, scalar*vector.z );

}  

Vector3& Vector3::operator*=( Dfloat scalar )
{
    x *= scalar;
    y *= scalar;
    z *= scalar;

    return *this;

}  

Vector3 Vector3::operator/( Dfloat scalar )
{
    return Vector3( x/scalar, y/scalar, z/scalar );

}  

Vector3& Vector3::operator/=( Dfloat scalar )
{
    x /= scalar;
    y /= scalar;
    z /= scalar;

    return *this;

}   

Dfloat Vector3::Dot( const Vector3& vector ) const
{
    return (x*vector.x + y*vector.y + z*vector.z);

}   

Dfloat Dot( const Vector3& vector1, const Vector3& vector2 )
{
    return (vector1.x*vector2.x + vector1.y*vector2.y + vector1.z*vector2.z);

}   

Vector3 Vector3::Cross( const Vector3& vector ) const
{
    return Vector3( y*vector.z - z*vector.y,
                      z*vector.x - x*vector.z,
                      x*vector.y - y*vector.x );

}  

Vector3 Cross( const Vector3& vector1, const Vector3& vector2 )
{
    return Vector3( vector1.y*vector2.z - vector1.z*vector2.y,
                      vector1.z*vector2.x - vector1.x*vector2.z,
                      vector1.x*vector2.y - vector1.y*vector2.x );

} 


