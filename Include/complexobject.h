#ifndef R5RS_COMPLEX_OBJECT_H
#define R5RS_COMPLEX_OBJECT_H

#include "object.h"

struct complex_object {
	Object ob_base;
	double re, im;
};

ComplexObject *Complex_New(double, double);

ComplexObject *Complex_FromFloat(FloatObject *);

ComplexObject *Complex_FromFraction(FractionObject *);

ComplexObject *Complex_FromLong(LongObject *);

ComplexObject *Complex_FromExpForm(Object *, Object *);

ComplexObject *Complex_FromExpFormCDouble(double, double);

void Complex_Print(ComplexObject *, FILE *);

ComplexObject *Complex_Add(ComplexObject *, ComplexObject *);

ComplexObject *Complex_Sub(ComplexObject *, ComplexObject *);

ComplexObject *Complex_Mul(ComplexObject *, ComplexObject *);

ComplexObject *Complex_Div(ComplexObject *, ComplexObject *);

ComplexObject *Complex_Neg(ComplexObject *);

ComplexObject *Complex_Conjugate(ComplexObject *);

FloatObject *Complex_Abs(ComplexObject *);

int Complex_IsZero(ComplexObject *);

Object *Complex_ToExact(ComplexObject *);

int Complex_IsInteger(ComplexObject *);

int Complex_IsRational(ComplexObject *);

int Complex_Eqv(ComplexObject *, Object *);

int Complex_EQ(ComplexObject *, Object *);


#endif //R5RS_COMPLEX_OBJECT_H
