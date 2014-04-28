#pragma once

#include "CoolD_Math.h"
#include "CoolD_Vector3.h"
#include "CoolD_Vector4.h"

class Matrix33;
class Vector3;

class Matrix44
{
public:
    // constructor/destructor
    inline Matrix44() { Identity(); }
    inline ~Matrix44() {}
    explicit Matrix44( const Matrix33& matrix );
	explicit Matrix44( const Dfloat* matrix);
    
    // copy operations
    Matrix44(const Matrix44& other);
    Matrix44& operator=(const Matrix44& other);

    // accessors
    Dfloat &operator()(unsigned int i, unsigned int j);
    Dfloat operator()(unsigned int i, unsigned int j) const;

    // comparison
    Dbool operator==( const Matrix44& other ) const;
    Dbool operator!=( const Matrix44& other ) const;
    Dbool IsZero() const;
    Dbool IsIdentity() const;

    // manipulators
    Dvoid SetRows( const Vector4& row1, const Vector4& row2, 
                  const Vector4& row3, const Vector4& row4 ); 
    Dvoid GetRows( Vector4& row1, Vector4& row2, Vector4& row3, Vector4& row4 ); 

    Dvoid SetColumns( const Vector4& col1, const Vector4& col2, 
                     const Vector4& col3, const Vector4& col4 ); 
    Dvoid GetColumns( Vector4& col1, Vector4& col2, Vector4& col3, Vector4& col4 ); 

    Dvoid Clean();
    Dvoid Identity();

    Matrix44& AffineInverse();
    friend Matrix44 AffineInverse( const Matrix44& mat );

    Matrix44& Transpose();
    friend Matrix44 Transpose( const Matrix44& mat );
        
    // transformations
    Matrix44& Translation( const Vector3& xlate );
    Matrix44& Rotation( const Matrix33& matrix );
    Matrix44& Rotation( Dfloat zRotation, Dfloat yRotation, Dfloat xRotation );
    Matrix44& Rotation( const Vector3& axis, Dfloat angle );

    Matrix44& Scaling( const Vector3& scale );
	Matrix44& Scaling(const Dfloat xScale, const Dfloat yScale, const Dfloat zScale);

    Matrix44& RotationX( Dfloat angle );
    Matrix44& RotationY( Dfloat angle );
    Matrix44& RotationZ( Dfloat angle );

    Dvoid GetFixedAngles( Dfloat& zRotation, Dfloat& yRotation, Dfloat& xRotation );
    Dvoid GetAxisAngle( Vector3& axis, Dfloat& angle );

    // operators

    // addition and subtraction
    Matrix44 operator+( const Matrix44& other ) const;
    Matrix44& operator+=( const Matrix44& other );
    Matrix44 operator-( const Matrix44& other ) const;
    Matrix44& operator-=( const Matrix44& other );

    Matrix44 operator-() const;

    // multiplication
    Matrix44& operator*=( const Matrix44& matrix );
    Matrix44 operator*( const Matrix44& matrix ) const;

    // column vector multiplier
    Vector4 operator*( const Vector4& vector ) const;
    // row vector multiplier

    // scalar multiplication
    Matrix44& operator*=( Dfloat scalar );    
    Matrix44 operator*( Dfloat scalar ) const;

    // vector3 ops
    Vector3 Transform( const Vector3& point ) const;

    // point ops
    Vector3 TransformPoint( const Vector3& point ) const;

    // low-level data accessors - implementation-dependent
    operator Dfloat*() { return mV; }
    operator const Dfloat*() const { return mV; }

    // member variables
    
	union
	{	
		Dfloat mV[ 16 ];
		Dfloat A[ 4 ][ 4 ];		
	};

protected:

private:
};

inline Dfloat&
Matrix44::operator()(unsigned int i, unsigned int j)
{
   return mV[i + 4*j];

} 

inline Dfloat
Matrix44::operator()(unsigned int i, unsigned int j) const
{
   return mV[i + 4*j];

}  
