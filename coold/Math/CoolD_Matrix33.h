#pragma once

#include "CoolD_Math.h"

class Vector3;

class Matrix33
{
public:
    // constructor/destructor
    inline Matrix33() {}
    inline ~Matrix33() {}
    
    // copy operations
    Matrix33(const Matrix33& other);
    Matrix33& operator=(const Matrix33& other);

    // accessors
    inline Dfloat& operator()(unsigned int i, unsigned int j);
    inline Dfloat operator()(unsigned int i, unsigned int j) const;

    // comparison
    Dbool operator==( const Matrix33& other ) const;
    Dbool operator!=( const Matrix33& other ) const;
    Dbool IsZero() const;
    Dbool IsIdentity() const;

    // manipulators
    Dvoid SetRows( const Vector3& row1, const Vector3& row2, const Vector3& row3 ); 
    Dvoid GetRows( Vector3& row1, Vector3& row2, Vector3& row3 ) const; 
    Vector3 GetRow( unsigned int i ) const; 

    Dvoid SetColumns( const Vector3& col1, const Vector3& col2, const Vector3& col3 ); 
    Dvoid GetColumns( Vector3& col1, Vector3& col2, Vector3& col3 ) const; 
    Vector3 GetColumn( unsigned int i ) const; 

    Dvoid Clean();
    Dvoid Identity();

    Matrix33& Inverse();
    friend Matrix33 Inverse( const Matrix33& mat );

    Matrix33& Transpose();
    friend Matrix33 Transpose( const Matrix33& mat );

    // useful computations
    Matrix33 Adjoint() const;
    Dfloat Determinant() const;
    Dfloat Trace() const;
        
    // transformations
    Matrix33& Rotation( Dfloat zRotation, Dfloat yRotation, Dfloat xRotation );
    Matrix33& Rotation( const Vector3& axis, Dfloat angle );

    Matrix33& Scaling( const Vector3& scale );

    Matrix33& RotationX( Dfloat angle );
    Matrix33& RotationY( Dfloat angle );
    Matrix33& RotationZ( Dfloat angle );

    Dvoid GetFixedAngles( Dfloat& zRotation, Dfloat& yRotation, Dfloat& xRotation );
    Dvoid GetAxisAngle( Vector3& axis, Dfloat& angle );

    // operators

    // addition and subtraction
    Matrix33 operator+( const Matrix33& other ) const;
    Matrix33& operator+=( const Matrix33& other );
    Matrix33 operator-( const Matrix33& other ) const;
    Matrix33& operator-=( const Matrix33& other );

    Matrix33 operator-() const;

    // multiplication
    Matrix33& operator*=( const Matrix33& matrix );
    Matrix33 operator*( const Matrix33& matrix ) const;

    // column vector multiplier
    Vector3 operator*( const Vector3& vector ) const;
    // row vector multiplier
    friend Vector3 operator*( const Vector3& vector, const Matrix33& matrix );

    Matrix33& operator*=( Dfloat scalar );
    friend Matrix33 operator*( Dfloat scalar, const Matrix33& matrix );
    Matrix33 operator*( Dfloat scalar ) const;

    // low-level data accessors - implementation-dependent
    operator Dfloat*() { return mV; }
    operator const Dfloat*() const { return mV; }

    // member variables
    Dfloat mV[9];

protected:

private:
};

inline Dfloat&
Matrix33::operator()(unsigned int i, unsigned int j)
{
   return mV[i + 3*j];

}   // End of Matrix33::operator()()

inline Dfloat
Matrix33::operator()(unsigned int i, unsigned int j) const
{
   return mV[i + 3*j];

}   // End of Matrix33::operator()()

