#include "CoolD_Math.h"

static inline Dfloat PolynomialSinQuadrant(Dfloat a)
{
    // sin(a) ~ a - 0.16666 a^3 + 0.0083143 a^5 - 0.00018542 a^7 
    return a * ( 1.0f + a * a * (-0.16666f 
                                 + a * a * (0.0083143f 
                                            - a * a * 0.00018542f))); 
}

Dvoid FastSinCos( Dfloat a, Dfloat& sina, Dfloat& cosa )
{  
    Dbool bNegate = false;
    if (a < 0.0f)
    {
        a = -a;
        bNegate = true;
    }

    const Dfloat kTwoOverPI = 1.0f / kHalfPI;
    Dfloat DfloatA = kTwoOverPI * a;
    int intA = (int)DfloatA;

    const Dfloat kRationalHalfPI = 201 / 128.0f;
    const Dfloat kRemainderHalfPI = 4.8382679e-4f;

    DfloatA = (a - kRationalHalfPI * intA) - kRemainderHalfPI * intA;

     Dfloat DfloatAMinusHalfPi = (DfloatA - kRationalHalfPI) - kRemainderHalfPI;

    switch (intA & 3)
    {
    case 0: // 0 - Pi/2
        sina = PolynomialSinQuadrant(DfloatA);
        cosa = PolynomialSinQuadrant(-DfloatAMinusHalfPi);
        break;
    case 1: // Pi/2 - Pi
        sina = PolynomialSinQuadrant(-DfloatAMinusHalfPi);
        cosa = PolynomialSinQuadrant(-DfloatA);
        break;
    case 2: // Pi - 3Pi/2
        sina = PolynomialSinQuadrant(-DfloatA);
        cosa = PolynomialSinQuadrant(DfloatAMinusHalfPi);
        break;
    case 3: // 3Pi/2 - 2Pi
        sina = PolynomialSinQuadrant(DfloatAMinusHalfPi);
        cosa = PolynomialSinQuadrant(DfloatA);
        break;
    };

    if (bNegate)
        sina = -sina;

} 


