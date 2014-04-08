#include "CoolD_Vector2.h"

Vector2 Vector2::xAxis( 1.0f, 0.0f );
Vector2 Vector2::yAxis( 0.0f, 1.0f );
Vector2 Vector2::origin( 0.0f, 0.0f );
Vector2 Vector2::xy( 1.0f, 1.0f );

Dfloat Vector2::Length() const
{
    return Sqrt( x*x + y*y );

}   

Dfloat Vector2::LengthSquared() const
{
    return (x*x + y*y);

}   

Dbool Vector2::operator==( const Vector2& other ) const
{
    if ( ::AreEqual( other.x, x )
        && ::AreEqual( other.y, y ) )
        return true;

    return false;   
}  

Dbool Vector2::operator!=( const Vector2& other ) const
{
    if ( ::AreEqual( other.x, x )
        && ::AreEqual( other.y, y ) )
        return false;

    return true;
}   

Dbool Vector2::IsZero() const
{
    return ::IsZero(x*x + y*y);

}  

Dvoid Vector2::Clean()
{
    if ( ::IsZero( x ) )
        x = 0.0f;
    if ( ::IsZero( y ) )
        y = 0.0f;

}  

Dvoid Vector2::Normalize()
{
    Dfloat lengthsq = x*x + y*y;

    if ( ::IsZero( lengthsq ) )
    {
        Zero();
    }
    else
    {
        Dfloat factor = InvSqrt( lengthsq );
        x *= factor;
        y *= factor;
    }

}  

Vector2 Vector2::operator+( const Vector2& other ) const
{
    return Vector2( x + other.x, y + other.y );

}  

Vector2& operator+=( Vector2& self, const Vector2& other )
{
    self.x += other.x;
    self.y += other.y;

    return self;

}   

Vector2 Vector2::operator-( const Vector2& other ) const
{
    return Vector2( x - other.x, y - other.y );

}   

Vector2& operator-=( Vector2& self, const Vector2& other )
{
    self.x -= other.x;
    self.y -= other.y;

    return self;

}   

Vector2 Vector2::operator-() const
{
    return Vector2(-x, -y);
}    

Vector2 Vector2::operator*( Dfloat scalar )
{
    return Vector2( scalar*x, scalar*y );

}  

Vector2 operator*( Dfloat scalar, const Vector2& vector )
{
    return Vector2( scalar*vector.x, scalar*vector.y );

}  

Vector2& Vector2::operator*=( Dfloat scalar )
{
    x *= scalar;
    y *= scalar;

    return *this;

}   

Vector2 Vector2::operator/( Dfloat scalar )
{
    return Vector2( x/scalar, y/scalar );

}   

Vector2& Vector2::operator/=( Dfloat scalar )
{
    x /= scalar;
    y /= scalar;

    return *this;

}  

Dfloat Vector2::Dot( const Vector2& vector ) const
{
    return (x*vector.x + y*vector.y);

}  

Dfloat Dot( const Vector2& vector1, const Vector2& vector2 )
{
    return (vector1.x*vector2.x + vector1.y*vector2.y);

} 

Dfloat Vector2::PerpDot( const Vector2& vector ) const
{
    return (x*vector.y - y*vector.x);

}  

Dfloat PerpDot( const Vector2& vector1, const Vector2& vector2 )
{
    return (vector1.x*vector2.y - vector1.y*vector2.x);

} 
