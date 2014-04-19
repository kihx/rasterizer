#include "CoolD_Vector4.h"

Vector4 Vector4::xAxis( 1.0f, 0.0f, 0.0f, 0.0f );
Vector4 Vector4::yAxis( 0.0f, 1.0f, 0.0f, 0.0f );
Vector4 Vector4::zAxis( 0.0f, 0.0f, 1.0f, 0.0f );
Vector4 Vector4::wAxis( 0.0f, 0.0f, 0.0f, 1.0f );
Vector4 Vector4::origin( 0.0f, 0.0f, 0.0f, 0.0f );

Vector4::Vector4(const Vector4& other) : 
    x( other.x ),
    y( other.y ),
    z( other.z ),
    w( other.w )
{

}  
Vector4& Vector4::operator=(const Vector4& other)
{
    // if same object
    if ( this == &other )
        return *this;
        
    x = other.x;
    y = other.y;
    z = other.z;
    w = other.w;
    
    return *this;

}   

Dfloat Vector4::Length() const
{
    return Sqrt( x*x + y*y + z*z + w*w );

}   

Dfloat Vector4::LengthSquared() const
{
    return ( x*x + y*y + z*z + w*w );

}   

Dbool Vector4::operator==( const Vector4& other ) const
{
    if ( ::AreEqual( other.x, x )
        && ::AreEqual( other.y, y )
        && ::AreEqual( other.z, z )
        && ::AreEqual( other.w, w ) )
        return true;

    return false;   
}   

Dbool Vector4::operator!=( const Vector4& other ) const
{
    if ( ::AreEqual( other.x, x )
        && ::AreEqual( other.y, y )
        && ::AreEqual( other.z, z )
        && ::AreEqual( other.w, w ) )
        return false;

    return true;
}   

Dbool Vector4::IsZero() const
{
    return ::IsZero(x*x + y*y + z*z + w*w);

}   

Dbool Vector4::IsUnit() const
{
    return ::IsZero(1.0f - x*x - y*y - z*z - w*w);

}   

Dvoid Vector4::Clean()
{
    if ( ::IsZero( x ) )
        x = 0.0f;
    if ( ::IsZero( y ) )
        y = 0.0f;
    if ( ::IsZero( z ) )
        z = 0.0f;
    if ( ::IsZero( w ) )
        w = 0.0f;

}   

Dvoid Vector4::Normalize()
{
    Dfloat lengthsq = x*x + y*y + z*z + w*w;

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
        w *= factor;
    }

}  

Vector4 Vector4::operator+( const Vector4& other ) const
{
    return Vector4( x + other.x, y + other.y, z + other.z, w + other.w );

}   

Vector4& Vector4::operator+=( const Vector4& other )
{
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;

    return *this;

}   

Vector4 Vector4::operator-( const Vector4& other ) const
{
    return Vector4( x - other.x, y - other.y, z - other.z, w - other.w );

}   

Vector4& Vector4::operator-=( const Vector4& other )
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;

    return *this;

}   

Vector4 Vector4::operator*( Dfloat scalar )
{
    return Vector4( scalar*x, scalar*y, scalar*z,
                      scalar*w );

}   

Vector4 operator*( Dfloat scalar, const Vector4& vector )
{
    return Vector4( scalar*vector.x, scalar*vector.y, scalar*vector.z,
                      scalar*vector.w );

}   

Vector4& Vector4::operator*=( Dfloat scalar )
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;

    return *this;

}   

Vector4 Vector4::operator/(Dfloat scalar) const
{
	return Vector4(x / scalar, y / scalar, z / scalar, w / scalar);
}

Vector4& Vector4::operator/=( Dfloat scalar )
{
    x /= scalar;
    y /= scalar;
    z /= scalar;
    w /= scalar;

    return *this;

}   

Dfloat Vector4::Dot( const Vector4& vector ) const
{
    return (x*vector.x + y*vector.y + z*vector.z + w*vector.w);

}   


Dfloat Dot( const Vector4& vector1, const Vector4& vector2 )
{
    return (vector1.x*vector2.x + vector1.y*vector2.y + vector1.z*vector2.z
            + vector1.w*vector2.w);

}

Vector3 Vec4ToVec3(const Vector4& vector, Vector4::W_MODE value)
{
	if( value == Vector4::W_DIVIDE )
	{		
		Vector4 v4 = vector / vector.w;
		return Vector3(v4.x, v4.y, v4.z);
	}

	//if( value == W_IGNORE )
	return Vector3(vector.x, vector.y, vector.z);
}
