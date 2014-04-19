
#include "CoolD_Matrix33.h"
#include "CoolD_Vector3.h"

Matrix33::Matrix33(const Matrix33& other)
{
    mV[0] = other.mV[0];
    mV[1] = other.mV[1];
    mV[2] = other.mV[2];
    mV[3] = other.mV[3];
    mV[4] = other.mV[4];
    mV[5] = other.mV[5];
    mV[6] = other.mV[6];
    mV[7] = other.mV[7];
    mV[8] = other.mV[8];
}

Matrix33& Matrix33::operator=(const Matrix33& other)
{
    if ( this == &other )
        return *this;
        
    mV[0] = other.mV[0];
    mV[1] = other.mV[1];
    mV[2] = other.mV[2];
    mV[3] = other.mV[3];
    mV[4] = other.mV[4];
    mV[5] = other.mV[5];
    mV[6] = other.mV[6];
    mV[7] = other.mV[7];
    mV[8] = other.mV[8];
    
    return *this;
} 


Dbool Matrix33::operator==( const Matrix33& other ) const
{
    for (unsigned int i = 0; i < 9; ++i)
    {
        if ( !AreEqual(mV[i], other.mV[i]) )
            return false;
    }
    return true;

} 

Dbool Matrix33::operator!=( const Matrix33& other ) const
{
    for (unsigned int i = 0; i < 9; ++i)
    {
        if ( !AreEqual(mV[i], other.mV[i]) )
            return true;
    }
    return false;

}  
Dbool Matrix33::IsZero() const
{
    for (unsigned int i = 0; i < 9; ++i)
    {
        if ( !::IsZero( mV[i] ) )
            return false;
    }
    return true;

}  

Dbool Matrix33::IsIdentity() const
{
    return ::AreEqual( 1.0f, mV[0] )
        && ::AreEqual( 1.0f, mV[4] )
        && ::AreEqual( 1.0f, mV[8] )
        && ::IsZero( mV[1] ) 
        && ::IsZero( mV[2] )
        && ::IsZero( mV[3] )
        && ::IsZero( mV[5] ) 
        && ::IsZero( mV[6] )
        && ::IsZero( mV[7] );

}   

Dvoid Matrix33::SetRows( const Vector3& row1, const Vector3& row2, const Vector3& row3 )
{
    mV[0] = row1.x;
    mV[3] = row1.y;
    mV[6] = row1.z;

    mV[1] = row2.x;
    mV[4] = row2.y;
    mV[7] = row2.z;

    mV[2] = row3.x;
    mV[5] = row3.y;
    mV[8] = row3.z;

}  

Dvoid Matrix33::GetRows( Vector3& row1, Vector3& row2, Vector3& row3 ) const
{
    row1.x = mV[0];
    row1.y = mV[3];
    row1.z = mV[6];

    row2.x = mV[1];
    row2.y = mV[4];
    row2.z = mV[7];

    row3.x = mV[2];
    row3.y = mV[5];
    row3.z = mV[8];
} 

Vector3 Matrix33::GetRow( unsigned int i ) const
{
    assert( i < 3 );
    return Vector3( mV[i], mV[i+3], mV[i+6] );

}  

Dvoid Matrix33::SetColumns( const Vector3& col1, const Vector3& col2, const Vector3& col3 )
{
    mV[0] = col1.x;
    mV[1] = col1.y;
    mV[2] = col1.z;

    mV[3] = col2.x;
    mV[4] = col2.y;
    mV[5] = col2.z;

    mV[6] = col3.x;
    mV[7] = col3.y;
    mV[8] = col3.z;

}  

Dvoid Matrix33::GetColumns( Vector3& col1, Vector3& col2, Vector3& col3 ) const
{
    col1.x = mV[0];
    col1.y = mV[1];
    col1.z = mV[2];

    col2.x = mV[3];
    col2.y = mV[4];
    col2.z = mV[5];

    col3.x = mV[6];
    col3.y = mV[7];
    col3.z = mV[8];

} 

Vector3 Matrix33::GetColumn( unsigned int i ) const 
{
    assert( i < 3 );
    return Vector3( mV[3*i], mV[3*i+1], mV[3*i+2] );

}   

Dvoid Matrix33::Clean()
{
    for (unsigned int i = 0; i < 9; ++i)
    {
        if ( ::IsZero( mV[i] ) )
            mV[i] = 0.0f;
    }

}   

Dvoid Matrix33::Identity()
{
    mV[0] = 1.0f;
    mV[1] = 0.0f;
    mV[2] = 0.0f;
    mV[3] = 0.0f;
    mV[4] = 1.0f;
    mV[5] = 0.0f;
    mV[6] = 0.0f;
    mV[7] = 0.0f;
    mV[8] = 1.0f;

} 

Matrix33& Matrix33::Inverse()
{
    *this = ::Inverse(*this);

    return *this;

}  

Matrix33 Inverse( const Matrix33& mat )
{
    Matrix33 result;
    
    // compute determinant
    Dfloat cofactor0 = mat.mV[4]*mat.mV[8] - mat.mV[5]*mat.mV[7];
    Dfloat cofactor3 = mat.mV[2]*mat.mV[7] - mat.mV[1]*mat.mV[8];
    Dfloat cofactor6 = mat.mV[1]*mat.mV[5] - mat.mV[2]*mat.mV[4];
    Dfloat det = mat.mV[0]*cofactor0 + mat.mV[3]*cofactor3 + mat.mV[6]*cofactor6;
    if (::IsZero( det ))
    {
        assert( false );        
        return result;
    }

    // create adjoint matrix and multiply by 1/det to get inverse
    Dfloat invDet = 1.0f/det;
    result.mV[0] = invDet*cofactor0;
    result.mV[1] = invDet*cofactor3;
    result.mV[2] = invDet*cofactor6;
   
    result.mV[3] = invDet*(mat.mV[5]*mat.mV[6] - mat.mV[3]*mat.mV[8]);
    result.mV[4] = invDet*(mat.mV[0]*mat.mV[8] - mat.mV[2]*mat.mV[6]);
    result.mV[5] = invDet*(mat.mV[2]*mat.mV[3] - mat.mV[0]*mat.mV[5]);

    result.mV[6] = invDet*(mat.mV[3]*mat.mV[7] - mat.mV[4]*mat.mV[6]);
    result.mV[7] = invDet*(mat.mV[1]*mat.mV[6] - mat.mV[0]*mat.mV[7]);
    result.mV[8] = invDet*(mat.mV[0]*mat.mV[4] - mat.mV[1]*mat.mV[3]);

    return result;

} 

Matrix33& Matrix33::Transpose()
{
    Dfloat temp = mV[1];
    mV[1] = mV[3];
    mV[3] = temp;

    temp = mV[2];
    mV[2] = mV[6];
    mV[6] = temp;

    temp = mV[5];
    mV[5] = mV[7];
    mV[7] = temp;

    return *this;

}  

Matrix33 Transpose( const Matrix33& mat )
{
    Matrix33 result;

    result.mV[0] = mat.mV[0];
    result.mV[1] = mat.mV[3];
    result.mV[2] = mat.mV[6];
    result.mV[3] = mat.mV[1];
    result.mV[4] = mat.mV[4];
    result.mV[5] = mat.mV[7];
    result.mV[6] = mat.mV[2];
    result.mV[7] = mat.mV[5];
    result.mV[8] = mat.mV[8];

    return result;

}  


Dfloat Matrix33::Determinant() const
{
    return mV[0]*(mV[4]*mV[8] - mV[5]*mV[7]) 
         + mV[3]*(mV[2]*mV[7] - mV[1]*mV[8])
         + mV[6]*(mV[1]*mV[5] - mV[2]*mV[4]);

}   

Matrix33 Matrix33::Adjoint() const
{
    Matrix33 result;
    
    // compute transpose of cofactors
    result.mV[0] = mV[4]*mV[8] - mV[5]*mV[7];
    result.mV[1] = mV[2]*mV[7] - mV[1]*mV[8];
    result.mV[2] = mV[1]*mV[5] - mV[2]*mV[4];
   
    result.mV[3] = mV[5]*mV[6] - mV[3]*mV[8];
    result.mV[4] = mV[0]*mV[8] - mV[2]*mV[6];
    result.mV[5] = mV[2]*mV[3] - mV[0]*mV[5];

    result.mV[6] = mV[3]*mV[7] - mV[4]*mV[6];
    result.mV[7] = mV[1]*mV[6] - mV[0]*mV[7];
    result.mV[8] = mV[0]*mV[4] - mV[1]*mV[3];

    return result;

}  


Dfloat Matrix33::Trace() const
{
    return mV[0] + mV[4] + mV[8];

}   

Matrix33& Matrix33::Rotation( Dfloat zRotation, Dfloat yRotation, Dfloat xRotation )
{
    // This is an "unrolled" contatenation of rotation matrices X Y & Z
    Dfloat Cx, Sx;
    SinCos(xRotation, Sx, Cx);

    Dfloat Cy, Sy;
    SinCos(yRotation, Sy, Cy);

    Dfloat Cz, Sz;
    SinCos(zRotation, Sz, Cz);

    mV[0] =  (Cy * Cz);
    mV[3] = -(Cy * Sz);  
    mV[6] =  Sy;

    mV[1] =  (Sx * Sy * Cz) + (Cx * Sz);
    mV[4] = -(Sx * Sy * Sz) + (Cx * Cz);
    mV[7] = -(Sx * Cy); 

    mV[2] = -(Cx * Sy * Cz) + (Sx * Sz);
    mV[5] =  (Cx * Sy * Sz) + (Sx * Cz);
    mV[8] =  (Cx * Cy);

    return *this;

} 

Matrix33& Matrix33::Rotation( const Vector3& axis, Dfloat angle )
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
    mV[3] = txy - sz;
    mV[6] = txz + sy;
    mV[1] = txy + sz;
    mV[4] = ty*nAxis.y + c;
    mV[7] = tyz - sx;
    mV[2] = txz - sy;
    mV[5] = tyz + sx;
    mV[8] = tz*nAxis.z + c;

    return *this;

} 

Matrix33& Matrix33::Scaling( const Vector3& scaleFactors )
{
    mV[0] = scaleFactors.x;
    mV[1] = 0.0f;
    mV[2] = 0.0f;
    mV[3] = 0.0f;
    mV[4] = scaleFactors.y;
    mV[5] = 0.0f;
    mV[6] = 0.0f;
    mV[7] = 0.0f;
    mV[8] = scaleFactors.z;

    return *this;

}  

Matrix33& Matrix33::RotationX( Dfloat angle )
{
    Dfloat sintheta, costheta;
    SinCos(angle, sintheta, costheta);

    mV[0] = 1.0f;
    mV[1] = 0.0f;
    mV[2] = 0.0f;
    mV[3] = 0.0f;
    mV[4] = costheta;
    mV[5] = sintheta;
    mV[6] = 0.0f;
    mV[7] = -sintheta;
    mV[8] = costheta;

    return *this;

}   

Matrix33& Matrix33::RotationY( Dfloat angle )
{
    Dfloat sintheta, costheta;
    SinCos(angle, sintheta, costheta);

    mV[0] = costheta;
    mV[1] = 0.0f;
    mV[2] = -sintheta;
    mV[3] = 0.0f;
    mV[4] = 1.0f;
    mV[5] = 0.0f;
    mV[6] = sintheta;
    mV[7] = 0.0f;
    mV[8] = costheta;

    return *this;

}   

Matrix33& Matrix33::RotationZ( Dfloat angle )
{
    Dfloat sintheta, costheta;
    SinCos(angle, sintheta, costheta);

    mV[0] = costheta;
    mV[1] = sintheta;
    mV[2] = 0.0f;
    mV[3] = -sintheta;
    mV[4] = costheta;
    mV[5] = 0.0f;
    mV[6] = 0.0f;
    mV[7] = 0.0f;
    mV[8] = 1.0f;

    return *this;

}  

Dvoid Matrix33::GetFixedAngles( Dfloat& zRotation, Dfloat& yRotation, Dfloat& xRotation )
{
    Dfloat Cx, Sx;
    Dfloat Cy, Sy;
    Dfloat Cz, Sz;

    Sy = mV[6];
    Cy = ::Sqrt( 1.0f - Sy*Sy );
    // normal case
    if ( !::IsZero( Cy ) )
    {
        Dfloat factor = 1.0f/Cy;
        Sx = -mV[7]*factor;
        Cx = mV[8]*factor;
        Sz = -mV[3]*factor;
        Cz = mV[0]*factor;
    }
    // x and z axes aligned
    else
    {
        Sz = 0.0f;
        Cz = 1.0f;
        Sx = mV[5];
        Cx = mV[4];
    }

    zRotation = atan2f( Sz, Cz );
    yRotation = atan2f( Sy, Cy );
    xRotation = atan2f( Sx, Cx );

} 

Dvoid Matrix33::GetAxisAngle( Vector3& axis, Dfloat& angle )
{
    Dfloat trace = mV[0] + mV[4] + mV[8];
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
        axis.Set( mV[5]-mV[7], mV[6]-mV[2], mV[1]-mV[3] );
        axis.Normalize();
    }
    // angle is 180 degrees
    else
    {
        unsigned int i = 0;
        if ( mV[4] > mV[0] )
            i = 1;
        if ( mV[8] > mV[i + 3*i] )
            i = 2;
        unsigned int j = (i+1)%3;
        unsigned int k = (j+1)%3;
        Dfloat s = ::Sqrt( mV[i + 3*i] - mV[j + 3*j] - mV[k + 3*k] + 1.0f );
        axis[i] = 0.5f*s;
        Dfloat recip = 1.0f/s;
        axis[j] = (mV[i + 3*j])*recip;
        axis[k] = (mV[k + 3*i])*recip;
    }

} 

Matrix33 Matrix33::operator+( const Matrix33& other ) const
{
    Matrix33 result;

    for (unsigned int i = 0; i < 9; ++i)
    {
        result.mV[i] = mV[i] + other.mV[i];
    }

    return result;

}  

Matrix33& Matrix33::operator+=( const Matrix33& other )
{
    for (unsigned int i = 0; i < 9; ++i)
    {
        mV[i] += other.mV[i];
    }

    return *this;

}   

Matrix33 Matrix33::operator-( const Matrix33& other ) const
{
    Matrix33 result;

    for (unsigned int i = 0; i < 9; ++i)
    {
        result.mV[i] = mV[i] - other.mV[i];
    }

    return result;

}   

Matrix33& Matrix33::operator-=( const Matrix33& other )
{
    for (unsigned int i = 0; i < 9; ++i)
    {
        mV[i] -= other.mV[i];
    }

    return *this;

}   

Matrix33 Matrix33::operator-() const
{
    Matrix33 result;

    for (unsigned int i = 0; i < 16; ++i)
    {
        result.mV[i] = -mV[i];
    }

    return result;

}    

Matrix33 Matrix33::operator*( const Matrix33& other ) const
{
    Matrix33 result;

    result.mV[0] = mV[0]*other.mV[0] + mV[3]*other.mV[1] + mV[6]*other.mV[2];
    result.mV[1] = mV[1]*other.mV[0] + mV[4]*other.mV[1] + mV[7]*other.mV[2];
    result.mV[2] = mV[2]*other.mV[0] + mV[5]*other.mV[1] + mV[8]*other.mV[2];
    result.mV[3] = mV[0]*other.mV[3] + mV[3]*other.mV[4] + mV[6]*other.mV[5];
    result.mV[4] = mV[1]*other.mV[3] + mV[4]*other.mV[4] + mV[7]*other.mV[5];
    result.mV[5] = mV[2]*other.mV[3] + mV[5]*other.mV[4] + mV[8]*other.mV[5];
    result.mV[6] = mV[0]*other.mV[6] + mV[3]*other.mV[7] + mV[6]*other.mV[8];
    result.mV[7] = mV[1]*other.mV[6] + mV[4]*other.mV[7] + mV[7]*other.mV[8];
    result.mV[8] = mV[2]*other.mV[6] + mV[5]*other.mV[7] + mV[8]*other.mV[8];

    return result;

}  

Matrix33& Matrix33::operator*=( const Matrix33& other )
{
    Matrix33 result;

    result.mV[0] = mV[0]*other.mV[0] + mV[3]*other.mV[1] + mV[6]*other.mV[2];
    result.mV[1] = mV[1]*other.mV[0] + mV[4]*other.mV[1] + mV[7]*other.mV[2];
    result.mV[2] = mV[2]*other.mV[0] + mV[5]*other.mV[1] + mV[8]*other.mV[2];
    result.mV[3] = mV[0]*other.mV[3] + mV[3]*other.mV[4] + mV[6]*other.mV[5];
    result.mV[4] = mV[1]*other.mV[3] + mV[4]*other.mV[4] + mV[7]*other.mV[5];
    result.mV[5] = mV[2]*other.mV[3] + mV[5]*other.mV[4] + mV[8]*other.mV[5];
    result.mV[6] = mV[0]*other.mV[6] + mV[3]*other.mV[7] + mV[6]*other.mV[8];
    result.mV[7] = mV[1]*other.mV[6] + mV[4]*other.mV[7] + mV[7]*other.mV[8];
    result.mV[8] = mV[2]*other.mV[6] + mV[5]*other.mV[7] + mV[8]*other.mV[8];

    for (unsigned int i = 0; i < 9; ++i)
    {
        mV[i] = result.mV[i];
    }

    return *this;

}   

Vector3 Matrix33::operator*( const Vector3& other ) const
{
    Vector3 result;

    result.x = mV[0]*other.x + mV[3]*other.y + mV[6]*other.z;
    result.y = mV[1]*other.x + mV[4]*other.y + mV[7]*other.z;
    result.z = mV[2]*other.x + mV[5]*other.y + mV[8]*other.z;

    return result;

}   

Vector3 operator*( const Vector3& vector, const Matrix33& mat )
{
    Vector3 result;

    result.x = mat.mV[0]*vector.x + mat.mV[1]*vector.y + mat.mV[2]*vector.z;
    result.y = mat.mV[3]*vector.x + mat.mV[4]*vector.y + mat.mV[5]*vector.z;
    result.z = mat.mV[6]*vector.x + mat.mV[7]*vector.y + mat.mV[8]*vector.z;

    return result;

}   

Matrix33& Matrix33::operator*=( Dfloat scalar )
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

    return *this;
}  

Matrix33 operator*( Dfloat scalar, const Matrix33& matrix )
{
    Matrix33 result;
    result.mV[0] = matrix.mV[0] * scalar;
    result.mV[1] = matrix.mV[1] * scalar;
    result.mV[2] = matrix.mV[2] * scalar;
    result.mV[3] = matrix.mV[3] * scalar;
    result.mV[4] = matrix.mV[4] * scalar;
    result.mV[5] = matrix.mV[5] * scalar;
    result.mV[6] = matrix.mV[6] * scalar;
    result.mV[7] = matrix.mV[7] * scalar;
    result.mV[8] = matrix.mV[8] * scalar;

    return result;
}  

Matrix33 Matrix33::operator*( Dfloat scalar ) const
{
    Matrix33 result;
    result.mV[0] = mV[0] * scalar;
    result.mV[1] = mV[1] * scalar;
    result.mV[2] = mV[2] * scalar;
    result.mV[3] = mV[3] * scalar;
    result.mV[4] = mV[4] * scalar;
    result.mV[5] = mV[5] * scalar;
    result.mV[6] = mV[6] * scalar;
    result.mV[7] = mV[7] * scalar;
    result.mV[8] = mV[8] * scalar;

    return result;
} 

