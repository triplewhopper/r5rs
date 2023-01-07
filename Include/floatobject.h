#ifndef R5RS_FLOAT_OBJECT_H
#define R5RS_FLOAT_OBJECT_H

#include "object.h"

struct float_object {
	Object ob_base;
	double ob_fval;
};


FloatObject *Float_New(double);

//FloatObject *Float_FromComplex(ComplexObject *);

FloatObject *Float_FromFraction(FractionObject *);

FloatObject *Float_FromLong(LongObject *);

FloatObject *Float_FromComplex(ComplexObject *);

Object *Float_AsIntegerRatio(FloatObject *);

//void Float_Dealloc(FloatObject *);

void Float_Print(FloatObject *, FILE *);

double Float_GetVal(FloatObject *);

FloatObject *Float_Add(FloatObject *, FloatObject *);

FloatObject *Float_Mul(FloatObject *, FloatObject *);

FloatObject *Float_MulByDouble(FloatObject *, double);

FloatObject *Float_Sub(FloatObject *, FloatObject *);

FloatObject *Float_Div(FloatObject *, FloatObject *);

FloatObject *Float_Neg(FloatObject *);

Object *Float_ToExact(FloatObject *);

int Float_Eqv(FloatObject *self, Object *other);

int Float_IsZero(FloatObject *);

int Float_IsInteger(FloatObject *);

int Float_IsRational(FloatObject *);

// return non-zero if and only if x is an NaN.
int Float_IsNaN(FloatObject *);

// return non-zero if and only if x is infinity.
int Float_IsINF(FloatObject *);

int Float_IsFinite(FloatObject *);

int Float_LT(FloatObject *self, Object *other);

int Float_EQ(FloatObject *self, Object *other);

int Float_NEQ(FloatObject *self, Object *other);

int Float_LE(FloatObject *self, Object *other);

int Float_GE(FloatObject *self, Object *other);

int Float_GT(FloatObject *self, Object *other);


#endif //R5RS_FLOAT_OBJECT_H
