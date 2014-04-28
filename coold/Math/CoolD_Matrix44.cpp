#include "CoolD_Matrix33.h"
#include "CoolD_Matrix44.h"
#include "CoolD_Vector3.h"

Matrix44::Matrix44(const Matrix44& other)
{
	for( int i = 0; i < 16; ++i )
	{
		mV[ i ] = other.mV[ i ];
	}
}  

Matrix44::Matrix44(const Matrix33& other)
{
    (Dvoid) Rotation( other );

}  

Matrix44::Matrix44(const Dfloat* matrix)
{
	for( int i = 0; i < 16; ++i )
	{
		mV[ i ] = matrix[ i ];
	}	
}

Matrix44& Matrix44::operator=(const Matrix44& other)
{
    // if same object
    if ( this == &other )
        return *this;
        
	for( int i = 0; i < 16; ++i )
	{
		mV[ i ] = other.mV[ i ];
	}
	
    return *this;
}   

Dbool Matrix44::operator==( const Matrix44& other ) const
{
    for (unsigned int i = 0; i < 16; ++i)
    {
        if ( !AreEqual(mV[i], other.mV[i]) )
            return false;
    }
    return true;

}  

Dbool Matrix44::operator!=( const Matrix44& other ) const
{
    for (unsigned int i = 0; i < 16; ++i)
    {
        if ( !AreEqual(mV[i], other.mV[i]) )
            return true;
    }
    return false;

}  

Dbool Matrix44::IsZero() const
{
    for (unsigned int i = 0; i < 16; ++i)
    {
        if ( !::IsZero( mV[i] ) )
            return false;
    }
    return true;

}   

Dbool Matrix44::IsIdentity() const
{
    return ::AreEqual( 1.0f, mV[0] )
        && ::AreEqual( 1.0f, mV[5] )
        && ::AreEqual( 1.0f, mV[10] )
        && ::AreEqual( 1.0f, mV[15] )
        && ::IsZero( mV[1] ) 
        && ::IsZero( mV[2] )
        && ::IsZero( mV[3] )
        && ::IsZero( mV[4] ) 
        && ::IsZero( mV[6] )
        && ::IsZero( mV[7] )
        && ::IsZero( mV[8] )
        && ::IsZero( mV[9] )
        && ::IsZero( mV[11] )
        && ::IsZero( mV[12] )
        && ::IsZero( mV[13] )
        && ::IsZero( mV[14] );

}   

Dvoid Matrix44::Clean()
{
    for (unsigned int i = 0; i < 16; ++i)
    {
        if ( ::IsZero( mV[i] ) )
            mV[i] = 0.0f;
    }

}  

Dvoid Matrix44::Identity()
{
    mV[0] = 1.0f;
    mV[1] = 0.0f;
    mV[2] = 0.0f;
    mV[3] = 0.0f;
    mV[4] = 0.0f;
    mV[5] = 1.0f;
    mV[6] = 0.0f;
    mV[7] = 0.0f;
    mV[8] = 0.0f;
    mV[9] = 0.0f;
    mV[10] = 1.0f;
    mV[11] = 0.0f;
    mV[12] = 0.0f;
    mV[13] = 0.0f;
    mV[14] = 0.0f;
    mV[15] = 1.0f;

}   

Matrix44& Matrix44::AffineInverse()
{
    *this = ::AffineInverse( *this );
    
    return *this;

}   

Matrix44 AffineInverse( const Matrix44& mat )
{
    Matrix44 result;
    
    // compute upper left 3x3 matrix determinant
    Dfloat cofactor0 = mat.mV[5]*mat.mV[10] - mat.mV[6]*mat.mV[9];
    Dfloat cofactor4 = mat.mV[2]*mat.mV[9] - mat.mV[1]*mat.mV[10];
    Dfloat cofactor8 = mat.mV[1]*mat.mV[6] - mat.mV[2]*mat.mV[5];
    Dfloat det = mat.mV[0]*cofactor0 + mat.mV[4]*cofactor4 + mat.mV[8]*cofactor8;
    if (::IsZero( det ))
    {
        assert( false );        
        return result;
    }

    // create adjunct matrix and multiply by 1/det to get upper 3x3
    Dfloat invDet = 1.0f/det;
    result.mV[0] = invDet*cofactor0;
    result.mV[1] = invDet*cofactor4;
    result.mV[2] = invDet*cofactor8;
   
    result.mV[4] = invDet*(mat.mV[6]*mat.mV[8] - mat.mV[4]*mat.mV[10]);
    result.mV[5] = invDet*(mat.mV[0]*mat.mV[10] - mat.mV[2]*mat.mV[8]);
    result.mV[6] = invDet*(mat.mV[2]*mat.mV[4] - mat.mV[0]*mat.mV[6]);

    result.mV[8] = invDet*(mat.mV[4]*mat.mV[9] - mat.mV[5]*mat.mV[8]);
    result.mV[9] = invDet*(mat.mV[1]*mat.mV[8] - mat.mV[0]*mat.mV[9]);
    result.mV[10] = invDet*(mat.mV[0]*mat.mV[5] - mat.mV[1]*mat.mV[4]);

    // multiply -translation by inverted 3x3 to get its inverse
    
    result.mV[12] = -result.mV[0]*mat.mV[12] - result.mV[4]*mat.mV[13] - result.mV[8]*mat.mV[14];
    result.mV[13] = -result.mV[1]*mat.mV[12] - result.mV[5]*mat.mV[13] - result.mV[9]*mat.mV[14];
    result.mV[14] = -result.mV[2]*mat.mV[12] - result.mV[6]*mat.mV[13] - result.mV[10]*mat.mV[14];

    return result;

}   

Matrix44& Matrix44::Transpose()
{
    Dfloat temp = mV[1];
    mV[1] = mV[4];
    mV[4] = temp;

    temp = mV[2];
    mV[2] = mV[8];
    mV[8] = temp;

    temp = mV[3];
    mV[3] = mV[12];
    mV[12] = temp;

    temp = mV[6];
    mV[6] = mV[9];
    mV[9] = temp;

    temp = mV[7];
    mV[7] = mV[13];
    mV[13] = temp;

    temp = mV[11];
    mV[11] = mV[14];
    mV[14] = temp;

    return *this;

}   

Matrix44 Transpose( const Matrix44& mat )
{
    Matrix44 result;

    result.mV[0] = mat.mV[0];
    result.mV[1] = mat.mV[4];
    result.mV[2] = mat.mV[8];
    result.mV[3] = mat.mV[12];
    result.mV[4] = mat.mV[1];
    result.mV[5] = mat.mV[5];
    result.mV[6] = mat.mV[9];
    result.mV[7] = mat.mV[13];
    result.mV[8] = mat.mV[2];
    result.mV[9] = mat.mV[6];
    result.mV[10] = mat.mV[10];
    result.mV[11] = mat.mV[14];
    result.mV[12] = mat.mV[3];
    result.mV[13] = mat.mV[7];
    result.mV[14] = mat.mV[11];
    result.mV[15] = mat.mV[15];

    return result;

}  

Matrix44& Matrix44::Translation( const Vector3& xlate )
{
    mV[0] = 1.0f;
    mV[1] = 0.0f;
    mV[2] = 0.0f;
    mV[3] = 0.0f;
    mV[4] = 0.0f;
    mV[5] = 1.0f;
    mV[6] = 0.0f;
    mV[7] = 0.0f;
    mV[8] = 0.0f;
    mV[9] = 0.0f;
    mV[10] = 1.0f;
    mV[11] = 0.0f;
    mV[12] = xlate.x;
    mV[13] = xlate.y;
    mV[14] = xlate.z;
    mV[15] = 1.0f;

    return *this;

}   
Matrix44& Matrix44::Rotation(const Matrix33& other)
{
    mV[0] = other.mV[0];
    mV[1] = other.mV[1];
    mV[2] = other.mV[2];
    mV[3] = 0;
    mV[4] = other.mV[3];
    mV[5] = other.mV[4];
    mV[6] = other.mV[5];
    mV[7] = 0;
    mV[8] = other.mV[6];
    mV[9] = other.mV[7];
    mV[10] = other.mV[8];
    mV[11] = 0;
    mV[12] = 0;
    mV[13] = 0;
    mV[14] = 0;
    mV[15] = 1;

    return *this;

}   

Matrix44& Matrix44::Rotation( Dfloat zRotation, Dfloat yRotation, Dfloat xRotation )
{
    // This is an "unrolled" contatenation of rotation matrices X Y & Z
    Dfloat Cx, Sx;
    SinCos(xRotation, Sx, Cx);

    Dfloat Cy, Sy;
    SinCos(yRotation, Sy, Cy);

    Dfloat Cz, Sz;
    SinCos(zRotation, Sz, Cz);

    mV[0] =  (Cy * Cz);
    mV[4] = -(Cy * Sz);  
    mV[8] =  Sy;
    mV[12] = 0.0f;

    mV[1] =  (Sx * Sy * Cz) + (Cx * Sz);
    mV[5] = -(Sx * Sy * Sz) + (Cx * Cz);
    mV[9] = -(Sx * Cy); 
    mV[13] = 0.0f;

    mV[2] = -(Cx * Sy * Cz) + (Sx * Sz);
    mV[6] =  (Cx * Sy * Sz) + (Sx * Cz);
    mV[10] =  (Cx * Cy);
    mV[14] = 0.0f;

    mV[3] = 0.0f;
    mV[7] = 0.0f;
    mV[11] = 0.0f;
    mV[15] = 1.0f;

    return *this;

}  

Matrix44& Matrix44::Rotation( const Vector3& axis, Dfloat angle )
{
    Dfloat c, s;
    SinCos(angle, s, c);
    Dfloat t = 1.0f - c;

    Vector3 nAxis = axis;
    nAxis.Normalize();

    // intermediate values
    Dfloat tx = t*nAxis.x;  Dfloat ty = t*nAxis.y;  Dfloat tz = t*nAxis.z;
    Dfloat sx = s*nAxis.x;  Dfloat sy = s*nAxis.y;  Dfloat sz = s*nAxis.z;
    Dfloat txy = tx*nAxis.y; Dfloat tyz = tx*nAxis.z; Dfloat txz = tx*nAxis.z;

    // set matrix
    mV[0] = tx*nAxis.x + c;
    mV[4] = txy - sz; 
    mV[8] = txz + sy;
    mV[12] = 0.0f;

    mV[1] = txy + sz;
    mV[5] = ty*nAxis.y + c;
    mV[9] = tyz - sx;
    mV[13] = 0.0f;

    mV[2] = txz - sy;
    mV[6] = tyz + sx;
    mV[10] = tz*nAxis.z + c;
    mV[14] = 0.0f;

    mV[3] = 0.0f;
    mV[7] = 0.0f;
    mV[11] = 0.0f;
    mV[15] = 1.0f;

    return *this;

}  

Matrix44& Matrix44::Scaling( const Vector3& scaleFactors )
{
	mV[ 0 ] = scaleFactors.x;
	mV[ 1 ] = 0.0f;
	mV[ 2 ] = 0.0f;
	mV[ 3 ] = 0.0f;
	mV[ 4 ] = 0.0f;
	mV[ 5 ] = scaleFactors.y;
	mV[ 6 ] = 0.0f;
	mV[ 7 ] = 0.0f;
	mV[ 8 ] = 0.0f;
	mV[ 9 ] = 0.0f;
	mV[ 10 ] = scaleFactors.z;
	mV[ 11 ] = 0.0f;
	mV[ 12 ] = 0.0f;
	mV[ 13 ] = 0.0f;
	mV[ 14 ] = 0.0f;
	mV[ 15 ] = 1.0f;

	return *this;
}   

Matrix44& Matrix44::Scaling(const Dfloat xScale, const Dfloat yScale, const Dfloat zScale)
{
	mV[ 0 ] = xScale;
	mV[ 1 ] = 0.0f;
	mV[ 2 ] = 0.0f;
	mV[ 3 ] = 0.0f;
	mV[ 4 ] = 0.0f;
	mV[ 5 ] = yScale;
	mV[ 6 ] = 0.0f;
	mV[ 7 ] = 0.0f;
	mV[ 8 ] = 0.0f;
	mV[ 9 ] = 0.0f;
	mV[ 10 ] = zScale;
	mV[ 11 ] = 0.0f;
	mV[ 12 ] = 0.0f;
	mV[ 13 ] = 0.0f;
	mV[ 14 ] = 0.0f;
	mV[ 15 ] = 1.0f;

	return *this;
}

Matrix44& Matrix44::RotationX( Dfloat angle )
{
    Dfloat sintheta, costheta;
    SinCos(angle, sintheta, costheta);

    mV[0] = 1.0f;
    mV[1] = 0.0f;
    mV[2] = 0.0f;
    mV[3] = 0.0f;
    mV[4] = 0.0f;
    mV[5] = costheta;
    mV[6] = sintheta;
    mV[7] = 0.0f;
    mV[8] = 0.0f;
    mV[9] = -sintheta;
    mV[10] = costheta;
    mV[11] = 0.0f;
    mV[12] = 0.0f;
    mV[13] = 0.0f;
    mV[14] = 0.0f;
    mV[15] = 1.0f;

    return *this;

}   

Matrix44& Matrix44::RotationY( Dfloat angle )
{
    Dfloat sintheta, costheta;
    SinCos(angle, sintheta, costheta);

    mV[0] = costheta;
    mV[1] = 0.0f;
    mV[2] = -sintheta;
    mV[3] = 0.0f;
    mV[4] = 0.0f;
    mV[5] = 1.0f;
    mV[6] = 0.0f;
    mV[7] = 0.0f;
    mV[8] = sintheta;
    mV[9] = 0.0f;
    mV[10] = costheta;
    mV[11] = 0.0f;
    mV[12] = 0.0f;
    mV[13] = 0.0f;
    mV[14] = 0.0f;
    mV[15] = 1.0f;    

    return *this;

}   

Matrix44& Matrix44::RotationZ( Dfloat angle )
{
    Dfloat sintheta, costheta;
    SinCos(angle, sintheta, costheta);

    mV[0] = costheta;
    mV[1] = sintheta;
    mV[2] = 0.0f;
    mV[3] = 0.0f;
    mV[4] = -sintheta;
    mV[5] = costheta;
    mV[6] = 0.0f;
    mV[7] = 0.0f;
    mV[8] = 0.0f;
    mV[9] = 0.0f;
    mV[10] = 1.0f;
    mV[11] = 0.0f;
    mV[12] = 0.0f;
    mV[13] = 0.0f;
    mV[14] = 0.0f;
    mV[15] = 1.0f;

    return *this;

}   

Dvoid Matrix44::GetFixedAngles( Dfloat& zRotation, Dfloat& yRotation, Dfloat& xRotation )
{
    Dfloat Cx, Sx;
    Dfloat Cy, Sy;
    Dfloat Cz, Sz;

    Sy = mV[8];
    Cy = ::Sqrt( 1.0f - Sy*Sy );
    // normal case
    if ( !::IsZero( Cy ) )
    {
        Dfloat factor = 1.0f/Cy;
        Sx = -mV[9]*factor;
        Cx = mV[10]*factor;
        Sz = -mV[4]*factor;
        Cz = mV[0]*factor;
    }
    // x and z axes aligned
    else
    {
        Sz = 0.0f;
        Cz = 1.0f;
        Sx = mV[6];
        Cx = mV[5];
    }

    zRotation = atan2f( Sz, Cz );
    yRotation = atan2f( Sy, Cy );
    xRotation = atan2f( Sx, Cx );

}  

Dvoid Matrix44::GetAxisAngle( Vector3& axis, Dfloat& angle )
{
    Dfloat trace = mV[0] + mV[5] + mV[10];
    Dfloat cosTheta = 0.5f*(trace - 1.0f);
    angle = acosf( cosTheta );

    // angle is zero, axis can be anything
    if ( ::IsZero( angle ) )
    {
        axis = Vector3::xAxis;
    }
    // standard case
    else if ( angle < kPI-kEpsilon )
    {
        axis.Set( mV[6]-mV[9], mV[8]-mV[2], mV[1]-mV[4] );
        axis.Normalize();
    }
    // angle is 180 degrees
    else
    {
        unsigned int i = 0;
        if ( mV[5] > mV[0] )
            i = 1;
        if ( mV[10] > mV[i + 4*i] )
            i = 2;
        unsigned int j = (i+1)%3;
        unsigned int k = (j+1)%3;
        Dfloat s = ::Sqrt( mV[i + 4*i] - mV[j + 4*j] - mV[k + 4*k] + 1.0f );
        axis[i] = 0.5f*s;
        Dfloat recip = 1.0f/s;
        axis[j] = (mV[i + 4*j])*recip;
        axis[k] = (mV[k + 4*i])*recip;
    }

}  

Matrix44 Matrix44::operator+( const Matrix44& other ) const
{
    Matrix44 result;

    for (unsigned int i = 0; i < 16; ++i)
    {
        result.mV[i] = mV[i] + other.mV[i];
    }

    return result;

}   

Matrix44& Matrix44::operator+=( const Matrix44& other )
{
    for (unsigned int i = 0; i < 16; ++i)
    {
        mV[i] += other.mV[i];
    }

    return *this;

}  

Matrix44 Matrix44::operator-( const Matrix44& other ) const
{
    Matrix44 result;

    for (unsigned int i = 0; i < 16; ++i)
    {
        result.mV[i] = mV[i] - other.mV[i];
    }

    return result;

}   

Matrix44& Matrix44::operator-=( const Matrix44& other )
{
    for (unsigned int i = 0; i < 16; ++i)
    {
        mV[i] -= other.mV[i];
    }

    return *this;

}   

Matrix44 Matrix44::operator-() const
{
    Matrix44 result;

    for (unsigned int i = 0; i < 16; ++i)
    {
        result.mV[i] = -mV[i];
    }

    return result;

}   

Matrix44 Matrix44::operator*( const Matrix44& other ) const
{
    Matrix44 result;

    result.mV[0] = mV[0]*other.mV[0] + mV[4]*other.mV[1] + mV[8]*other.mV[2] + mV[12]*other.mV[3];
    result.mV[1] = mV[1]*other.mV[0] + mV[5]*other.mV[1] + mV[9]*other.mV[2] + mV[13]*other.mV[3];
    result.mV[2] = mV[2]*other.mV[0] + mV[6]*other.mV[1] + mV[10]*other.mV[2] + mV[14]*other.mV[3];
    result.mV[3] = mV[3]*other.mV[0] + mV[7]*other.mV[1] + mV[11]*other.mV[2] + mV[15]*other.mV[3];

    result.mV[4] = mV[0]*other.mV[4] + mV[4]*other.mV[5] + mV[8]*other.mV[6] + mV[12]*other.mV[7];
    result.mV[5] = mV[1]*other.mV[4] + mV[5]*other.mV[5] + mV[9]*other.mV[6] + mV[13]*other.mV[7];
    result.mV[6] = mV[2]*other.mV[4] + mV[6]*other.mV[5] + mV[10]*other.mV[6] + mV[14]*other.mV[7];
    result.mV[7] = mV[3]*other.mV[4] + mV[7]*other.mV[5] + mV[11]*other.mV[6] + mV[15]*other.mV[7];

    result.mV[8] = mV[0]*other.mV[8] + mV[4]*other.mV[9] + mV[8]*other.mV[10] + mV[12]*other.mV[11];
    result.mV[9] = mV[1]*other.mV[8] + mV[5]*other.mV[9] + mV[9]*other.mV[10] + mV[13]*other.mV[11];
    result.mV[10] = mV[2]*other.mV[8] + mV[6]*other.mV[9] + mV[10]*other.mV[10] + mV[14]*other.mV[11];
    result.mV[11] = mV[3]*other.mV[8] + mV[7]*other.mV[9] + mV[11]*other.mV[10] + mV[15]*other.mV[11];

    result.mV[12] = mV[0]*other.mV[12] + mV[4]*other.mV[13] + mV[8]*other.mV[14] + mV[12]*other.mV[15];
    result.mV[13] = mV[1]*other.mV[12] + mV[5]*other.mV[13] + mV[9]*other.mV[14] + mV[13]*other.mV[15];
    result.mV[14] = mV[2]*other.mV[12] + mV[6]*other.mV[13] + mV[10]*other.mV[14] + mV[14]*other.mV[15];
    result.mV[15] = mV[3]*other.mV[12] + mV[7]*other.mV[13] + mV[11]*other.mV[14] + mV[15]*other.mV[15];

    return result;

}   

Matrix44& Matrix44::operator*=( const Matrix44& other )
{
    Matrix44 result;

    result.mV[0] = mV[0]*other.mV[0] + mV[4]*other.mV[1] + mV[8]*other.mV[2] 
                    + mV[12]*other.mV[3];
    result.mV[1] = mV[1]*other.mV[0] + mV[5]*other.mV[1] + mV[9]*other.mV[2] 
                    + mV[13]*other.mV[3];
    result.mV[2] = mV[2]*other.mV[0] + mV[6]*other.mV[1] + mV[10]*other.mV[2] 
                    + mV[14]*other.mV[3];
    result.mV[3] = mV[3]*other.mV[0] + mV[7]*other.mV[1] + mV[11]*other.mV[2] 
                    + mV[15]*other.mV[3];

    result.mV[4] = mV[0]*other.mV[4] + mV[4]*other.mV[5] + mV[8]*other.mV[6] 
                    + mV[12]*other.mV[7];
    result.mV[5] = mV[1]*other.mV[4] + mV[5]*other.mV[5] + mV[9]*other.mV[6] 
                    + mV[13]*other.mV[7];
    result.mV[6] = mV[2]*other.mV[4] + mV[6]*other.mV[5] + mV[10]*other.mV[6] 
                    + mV[14]*other.mV[7];
    result.mV[7] = mV[3]*other.mV[4] + mV[7]*other.mV[5] + mV[11]*other.mV[6] 
                    + mV[15]*other.mV[7];

    result.mV[8] = mV[0]*other.mV[8] + mV[4]*other.mV[9] + mV[8]*other.mV[10] 
                    + mV[12]*other.mV[11];
    result.mV[9] = mV[1]*other.mV[8] + mV[5]*other.mV[9] + mV[9]*other.mV[10] 
                    + mV[13]*other.mV[11];
    result.mV[10] = mV[2]*other.mV[8] + mV[6]*other.mV[9] + mV[10]*other.mV[10] 
                    + mV[14]*other.mV[11];
    result.mV[11] = mV[3]*other.mV[8] + mV[7]*other.mV[9] + mV[11]*other.mV[10] 
                    + mV[15]*other.mV[11];

    result.mV[12] = mV[0]*other.mV[12] + mV[4]*other.mV[13] + mV[8]*other.mV[14] 
                    + mV[12]*other.mV[15];
    result.mV[13] = mV[1]*other.mV[12] + mV[5]*other.mV[13] + mV[9]*other.mV[14] 
                    + mV[13]*other.mV[15];
    result.mV[14] = mV[2]*other.mV[12] + mV[6]*other.mV[13] + mV[10]*other.mV[14] 
                    + mV[14]*other.mV[15];
    result.mV[15] = mV[3]*other.mV[12] + mV[7]*other.mV[13] + mV[11]*other.mV[14] 
                    + mV[15]*other.mV[15];

    for (unsigned int i = 0; i < 16; ++i)
    {
        mV[i] = result.mV[i];
    }

    return *this;

}   
Vector4 Matrix44::operator*( const Vector4& other ) const
{
    Vector4 result;

    result.x = mV[0]*other.x + mV[4]*other.y + mV[8]*other.z + mV[12]*other.w;
    result.y = mV[1]*other.x + mV[5]*other.y + mV[9]*other.z + mV[13]*other.w;

    result.z = mV[2]*other.x + mV[6]*other.y + mV[10]*other.z + mV[14]*other.w;
    result.w = mV[3]*other.x + mV[7]*other.y + mV[11]*other.z + mV[15]*other.w;

    return result;

}   

Vector4 operator*( const Vector4& vector, const Matrix44& matrix )
{
    Vector4 result;

    result.x = matrix.mV[0]*vector.x + matrix.mV[1]*vector.y 
             + matrix.mV[2]*vector.z + matrix.mV[3]*vector.w;
    result.y = matrix.mV[4]*vector.x + matrix.mV[5]*vector.y 
             + matrix.mV[6]*vector.z + matrix.mV[7]*vector.w;
    result.z = matrix.mV[8]*vector.x + matrix.mV[9]*vector.y 
             + matrix.mV[10]*vector.z + matrix.mV[11]*vector.w;
    result.w = matrix.mV[12]*vector.x + matrix.mV[13]*vector.y 
             + matrix.mV[14]*vector.z + matrix.mV[15]*vector.w;

    return result;

}  

Matrix44& Matrix44::operator*=( Dfloat scalar )
{
    mV[0] *= scalar;
    mV[1] *= scalar;
    mV[2] *= scalar;
    mV[3] *= scalar;
    mV[4] *= scalar;
    mV[5] *= scalar;
    mV[6] *= scalar;
    mV[7] *= scalar;
    mV[8] *= scalar;
    mV[9] *= scalar;
    mV[10] *= scalar;
    mV[11] *= scalar;
    mV[12] *= scalar;
    mV[13] *= scalar;
    mV[14] *= scalar;
    mV[15] *= scalar;

    return *this;
}  

Matrix44 operator*( Dfloat scalar, const Matrix44& matrix )
{
    Matrix44 result;
    result.mV[0] = matrix.mV[0] * scalar;
    result.mV[1] = matrix.mV[1] * scalar;
    result.mV[2] = matrix.mV[2] * scalar;
    result.mV[3] = matrix.mV[3] * scalar;
    result.mV[4] = matrix.mV[4] * scalar;
    result.mV[5] = matrix.mV[5] * scalar;
    result.mV[6] = matrix.mV[6] * scalar;
    result.mV[7] = matrix.mV[7] * scalar;
    result.mV[8] = matrix.mV[8] * scalar;
    result.mV[9] = matrix.mV[9] * scalar;
    result.mV[10] = matrix.mV[10] * scalar;
    result.mV[11] = matrix.mV[11] * scalar;
    result.mV[12] = matrix.mV[12] * scalar;
    result.mV[13] = matrix.mV[13] * scalar;
    result.mV[14] = matrix.mV[14] * scalar;
    result.mV[15] = matrix.mV[15] * scalar;

    return result;
}  

Matrix44 Matrix44::operator*( Dfloat scalar ) const
{
    Matrix44 result;
    result.mV[0] = mV[0] * scalar;
    result.mV[1] = mV[1] * scalar;
    result.mV[2] = mV[2] * scalar;
    result.mV[3] = mV[3] * scalar;
    result.mV[4] = mV[4] * scalar;
    result.mV[5] = mV[5] * scalar;
    result.mV[6] = mV[6] * scalar;
    result.mV[7] = mV[7] * scalar;
    result.mV[8] = mV[8] * scalar;
    result.mV[9] = mV[9] * scalar;
    result.mV[10] = mV[10] * scalar;
    result.mV[11] = mV[11] * scalar;
    result.mV[12] = mV[12] * scalar;
    result.mV[13] = mV[13] * scalar;
    result.mV[14] = mV[14] * scalar;
    result.mV[15] = mV[15] * scalar;

    return result;
}  

Vector3 Matrix44::Transform( const Vector3& other ) const
{
    Vector3 result;

    result.x = mV[0]*other.x + mV[4]*other.y + mV[8]*other.z + mV[12];
    result.y = mV[1]*other.x + mV[5]*other.y + mV[9]*other.z + mV[13];
    result.z = mV[2]*other.x + mV[6]*other.y + mV[10]*other.z + mV[14];
 
    return result;

}   
Vector3 Matrix44::TransformPoint( const Vector3& other ) const
{
    Vector3 result;

    result.x = mV[0]*other.x + mV[4]*other.y + mV[8]*other.z + mV[12];
    result.y = mV[1]*other.x + mV[5]*other.y + mV[9]*other.z + mV[13];
    result.z = mV[2]*other.x + mV[6]*other.y + mV[10]*other.z + mV[14];
 
    return result;

}  
