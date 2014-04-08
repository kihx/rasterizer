#include "CoolD_Quat.h"
#include "CoolD_Vector3.h"
#include "CoolD_Matrix33.h"

Quat Quat::zero( 0.0f, 0.0f, 0.0f, 0.0f );
Quat Quat::identity( 1.0f, 0.0f, 0.0f, 0.0f );

Quat::Quat( const Vector3& axis, Dfloat angle )
{
    Set( axis, angle );
}  

Quat::Quat( const Vector3& from, const Vector3& to )
{
    Set( from, to );
}  

Quat::Quat( const Vector3& vector )
{
    Set( 0.0f, vector.x, vector.y, vector.z );
}  
Quat::Quat( const Matrix33& rotation )
{
    Dfloat trace = rotation.Trace();
    if ( trace > 0.0f )
    {
        Dfloat s = ::Sqrt( trace + 1.0f );
        w = s*0.5f;
        Dfloat recip = 0.5f/s;
        x = (rotation(2,1) - rotation(1,2))*recip;
        y = (rotation(0,2) - rotation(2,0))*recip;
        z = (rotation(1,0) - rotation(0,1))*recip;
    }
    else
    {
        unsigned int i = 0;
        if ( rotation(1,1) > rotation(0,0) )
            i = 1;
        if ( rotation(2,2) > rotation(i,i) )
            i = 2;
        unsigned int j = (i+1)%3;
        unsigned int k = (j+1)%3;
        Dfloat s = ::Sqrt( rotation(i,i) - rotation(j,j) - rotation(k,k) + 1.0f );
        (*this)[i] = 0.5f*s;
        Dfloat recip = 0.5f/s;
        w = (rotation(k,j) - rotation(j,k))*recip;
        (*this)[j] = (rotation(j,i) + rotation(i,j))*recip;
        (*this)[k] = (rotation(k,i) + rotation(i,k))*recip;
    }

}  

Quat::Quat(const Quat& other) : 
    w( other.w ),
    x( other.x ),
    y( other.y ),
    z( other.z )
{

}  

Quat& Quat::operator=(const Quat& other)
{
    // if same object
    if ( this == &other )
        return *this;
        
    w = other.w;
    x = other.x;
    y = other.y;
    z = other.z;
    
    return *this;

}   

Dfloat Quat::Magnitude() const
{
    return Sqrt( w*w + x*x + y*y + z*z );

}   

Dfloat Quat::Norm() const
{
    return ( w*w + x*x + y*y + z*z );

}   

Dbool Quat::operator==( const Quat& other ) const
{
    if ( ::IsZero( other.w - w )
        && ::IsZero( other.x - x )
        && ::IsZero( other.y - y )
        && ::IsZero( other.z - z ) )
        return true;

    return false;   
}   

Dbool Quat::operator!=( const Quat& other ) const
{
    if ( ::IsZero( other.w - w )
        || ::IsZero( other.x - x )
        || ::IsZero( other.y - y )
        || ::IsZero( other.z - z ) )
        return false;

    return true;
}   

Dbool Quat::IsZero() const
{
    return ::IsZero(w*w + x*x + y*y + z*z);

}  

Dbool Quat::IsUnit() const
{
    return ::IsZero(1.0f - w*w - x*x - y*y - z*z);

}   

Dbool Quat::IsIdentity() const
{
    return ::IsZero(1.0f - w)
        && ::IsZero( x ) 
        && ::IsZero( y )
        && ::IsZero( z );

}  

Dvoid Quat::Set( const Vector3& axis, Dfloat angle )
{
    // if axis of rotation is zero vector, just set to identity quat
    Dfloat length = axis.LengthSquared();
    if ( ::IsZero( length ) )
    {
        Identity();
        return;
    }

    // take half-angle
    angle *= 0.5f;

    Dfloat sintheta, costheta;
    SinCos(angle, sintheta, costheta);

    Dfloat scaleFactor = sintheta/Sqrt( length );

    w = costheta;
    x = scaleFactor * axis.x;
    y = scaleFactor * axis.y;
    z = scaleFactor * axis.z;

}   

Dvoid Quat::Set( const Vector3& from, const Vector3& to )
{
   // get axis of rotation
    Vector3 axis = from.Cross( to );

    // get scaled cos of angle between vectors and set initial quaternion
    Set(  from.Dot( to ), axis.x, axis.y, axis.z );
    // quaternion at this point is ||from||*||to||*( cos(theta), r*sin(theta) )

    // normalize to remove ||from||*||to|| factor
    Normalize();
    // quaternion at this point is ( cos(theta), r*sin(theta) )
    // what we want is ( cos(theta/2), r*sin(theta/2) )

    // set up for half angle calculation
    w += 1.0f;

    // now when we normalize, we'll be dividing by sqrt(2*(1+cos(theta))), which is 
    // what we want for r*sin(theta) to give us r*sin(theta/2)  (see pages 487-488)
    // 
    // w will become 
    //                 1+cos(theta)
    //            ----------------------
    //            sqrt(2*(1+cos(theta)))        
    // which simplifies to
    //                cos(theta/2)

    // before we normalize, check if vectors are opposing
    if ( w <= kEpsilon )
    {
        // rotate pi radians around orthogonal vector
        // take cross product with x axis
        if ( from.z*from.z > from.x*from.x )
            Set( 0.0f, 0.0f, from.z, -from.y );
        // or take cross product with z axis
        else
            Set( 0.0f, from.y, -from.x, 0.0f );
    }
   
    // normalize again to get rotation quaternion
    Normalize();

}   

Dvoid Quat::Set( Dfloat zRotation, Dfloat yRotation, Dfloat xRotation ) 
{
    zRotation *= 0.5f;
    yRotation *= 0.5f;
    xRotation *= 0.5f;

    // get sines and cosines of half angles
    Dfloat Cx, Sx;
    SinCos(xRotation, Sx, Cx);

    Dfloat Cy, Sy;
    SinCos(yRotation, Sy, Cy);

    Dfloat Cz, Sz;
    SinCos(zRotation, Sz, Cz);

    // multiply it out
    w = Cx*Cy*Cz - Sx*Sy*Sz;
    x = Sx*Cy*Cz + Cx*Sy*Sz;
    y = Cx*Sy*Cz - Sx*Cy*Sz;
    z = Cx*Cy*Sz + Sx*Sy*Cx;

}  

Dvoid Quat::GetAxisAngle( Vector3& axis, Dfloat& angle )
{
    angle = 2.0f*acosf( w );
    Dfloat length = ::Sqrt( 1.0f - w*w );
    if ( ::IsZero(length) )
        axis.Zero();
    else
    {
        length = 1.0f/length;
        axis.Set( x*length, y*length, z*length );
    }

}   

Dvoid Quat::Clean()
{
    if ( ::IsZero( w ) )
        w = 0.0f;
    if ( ::IsZero( x ) )
        x = 0.0f;
    if ( ::IsZero( y ) )
        y = 0.0f;
    if ( ::IsZero( z ) )
        z = 0.0f;

}   

Dvoid Quat::Normalize()
{
    Dfloat lengthsq = w*w + x*x + y*y + z*z;

    if ( ::IsZero( lengthsq ) )
    {
        Zero();
    }
    else
    {
        Dfloat factor = InvSqrt( lengthsq );
        w *= factor;
        x *= factor;
        y *= factor;
        z *= factor;
    }

}  

Quat Conjugate( const Quat& quat ) 
{
    return Quat( quat.w, -quat.x, -quat.y, -quat.z );

}  

const Quat& Quat::Conjugate()
{
    x = -x;
    y = -y;
    z = -z;

    return *this;

}   

Quat Inverse( const Quat& quat )
{
    Dfloat norm = quat.w*quat.w + quat.x*quat.x + quat.y*quat.y + quat.z*quat.z;
    // if we're the zero quaternion, just return identity
    if ( !::IsZero( norm ) )
    {
        assert( false );
        return Quat();
    }

    Dfloat normRecip = 1.0f / norm;
    return Quat( normRecip*quat.w, -normRecip*quat.x, -normRecip*quat.y, 
                   -normRecip*quat.z );

}   

const Quat& Quat::Inverse()
{
    Dfloat norm = w*w + x*x + y*y + z*z;
    // if we're the zero quaternion, just return
    if ( ::IsZero( norm ) )
        return *this;

    Dfloat normRecip = 1.0f / norm;
    w = normRecip*w;
    x = -normRecip*x;
    y = -normRecip*y;
    z = -normRecip*z;

    return *this;

}   

Quat Quat::operator+( const Quat& other ) const
{
    return Quat( w + other.w, x + other.x, y + other.y, z + other.z );

}   

Quat& Quat::operator+=( const Quat& other )
{
    w += other.w;
    x += other.x;
    y += other.y;
    z += other.z;

    return *this;

}   

Quat Quat::operator-( const Quat& other ) const
{
    return Quat( w - other.w, x - other.x, y - other.y, z - other.z );

}   

Quat& Quat::operator-=( const Quat& other )
{
    w -= other.w;
    x -= other.x;
    y -= other.y;
    z -= other.z;

    return *this;

}   

Quat Quat::operator-() const
{
    return Quat(-w, -x, -y, -z);
}    

Quat operator*( Dfloat scalar, const Quat& quat )
{
    return Quat( scalar*quat.w, scalar*quat.x, scalar*quat.y, scalar*quat.z );

}   

Quat& Quat::operator*=( Dfloat scalar )
{
    w *= scalar;
    x *= scalar;
    y *= scalar;
    z *= scalar;

    return *this;

}   

Quat Quat::operator*( const Quat& other ) const
{
    return Quat( w*other.w - x*other.x - y*other.y - z*other.z,
                   w*other.x + x*other.w + y*other.z - z*other.y,
                   w*other.y + y*other.w + z*other.x - x*other.z,
                   w*other.z + z*other.w + x*other.y - y*other.x );

}   

Quat& Quat::operator*=( const Quat& other )
{
    Set( w*other.w - x*other.x - y*other.y - z*other.z,
         w*other.x + x*other.w + y*other.z - z*other.y,
         w*other.y + y*other.w + z*other.x - x*other.z,
         w*other.z + z*other.w + x*other.y - y*other.x );
  
    return *this;

}   

Dfloat Quat::Dot( const Quat& quat ) const
{
    return ( w*quat.w + x*quat.x + y*quat.y + z*quat.z);

}  

Dfloat Dot( const Quat& quat1, const Quat& quat2 )
{
    return (quat1.w*quat2.w + quat1.x*quat2.x + quat1.y*quat2.y + quat1.z*quat2.z);

}  

Vector3 Quat::Rotate( const Vector3& vector ) const
{
    assert( IsUnit() );

    Dfloat vMult = 2.0f*(x*vector.x + y*vector.y + z*vector.z);
    Dfloat crossMult = 2.0f*w;
    Dfloat pMult = crossMult*w - 1.0f;

    return Vector3( pMult*vector.x + vMult*x + crossMult*(y*vector.z - z*vector.y),
                      pMult*vector.y + vMult*y + crossMult*(z*vector.x - x*vector.z),
                      pMult*vector.z + vMult*z + crossMult*(x*vector.y - y*vector.x) );

}  

Dvoid Lerp( Quat& result, const Quat& start, const Quat& end, Dfloat t )
{
    // get cos of "angle" between quaternions
    Dfloat cosTheta = start.Dot( end );

    // initialize result
    result = t*end;

    // if "angle" between quaternions is less than 90 degrees
    if ( cosTheta >= kEpsilon )
    {
        // use standard interpolation
        result += (1.0f-t)*start;
    }
    else
    {
        // otherwise, take the shorter path
        result += (t-1.0f)*start;
    }

}  

Dvoid Slerp( Quat& result, const Quat& start, const Quat& end, Dfloat t )
{
    // get cosine of "angle" between quaternions
    Dfloat cosTheta = start.Dot( end );
    Dfloat startInterp, endInterp;

    // if "angle" between quaternions is less than 90 degrees
    if ( cosTheta >= kEpsilon )
    {
        // if angle is greater than zero
        if ( (1.0f - cosTheta) > kEpsilon )
        {
            // use standard slerp
            Dfloat theta = acosf( cosTheta );
            Dfloat recipSinTheta = 1.0f/Sin( theta );

            startInterp = Sin( (1.0f - t)*theta )*recipSinTheta;
            endInterp = Sin( t*theta )*recipSinTheta;
        }
        // angle is close to zero
        else
        {
            // use linear interpolation
            startInterp = 1.0f - t;
            endInterp = t;
        }
    }
    // otherwise, take the shorter route
    else
    {
        // if angle is less than 180 degrees
        if ( (1.0f + cosTheta) > kEpsilon )
        {
            // use slerp w/negation of start quaternion
            Dfloat theta = acosf( -cosTheta );
            Dfloat recipSinTheta = 1.0f/Sin( theta );

            startInterp = Sin( (t-1.0f)*theta )*recipSinTheta;
            endInterp = Sin( t*theta )*recipSinTheta;
        }
        // angle is close to 180 degrees
        else
        {
            // use lerp w/negation of start quaternion
            startInterp = t - 1.0f;
            endInterp = t;
        }
    }
    
    result = startInterp*start + endInterp*end;

}  

Dvoid ApproxSlerp( Quat& result, const Quat& start, const Quat& end, Dfloat t )
{
    Dfloat cosTheta = start.Dot( end );

    // correct time by using cosine of angle between quaternions
    Dfloat factor = 1.0f - 0.7878088f*cosTheta;
    Dfloat k = 0.5069269f;
    factor *= factor;
    k *= factor;

    Dfloat b = 2*k;
    Dfloat c = -3*k;
    Dfloat d = 1 + k;

    t = t*(b*t + c) + d;

    // initialize result
    result = t*end;

    // if "angle" between quaternions is less than 90 degrees
    if ( cosTheta >= kEpsilon )
    {
        // use standard interpolation
        result += (1.0f-t)*start;
    }
    else
    {
        // otherwise, take the shorter path
        result += (t-1.0f)*start;
    }

}   
