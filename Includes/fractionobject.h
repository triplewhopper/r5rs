#ifndef R5RS_FRACTION_OBJECT_H
#define R5RS_FRACTION_OBJECT_H

#include "object.h"

struct fraction_object {
	Object ob_base;
	LongObject *_numerator;
	LongObject *_denominator;
	/*
	 * Invariants:
	 * for normalized:
	 * _denominator > 0
	 * gcd(_numerator, _denominator) == 1
	 */
};

FractionObject *Fraction_New(LongObject *, LongObject *, int);

FractionObject *Fraction_FromLong(LongObject *);

FractionObject *Fraction_FromFloat(FloatObject *);

FractionObject *Fraction_FromComplex(ComplexObject *);

void Fraction_Dealloc(FractionObject *);

void Fraction_Search(FractionObject *self, Object *target, ArrayObject *res);

void Fraction_Print(FractionObject *, FILE *);

LongObject *Fraction_GetNumerator(FractionObject *);

LongObject *Fraction_GetDenominator(FractionObject *);

FractionObject *Fraction_Add(FractionObject *, FractionObject *);

FractionObject *Fraction_Sub(FractionObject *, FractionObject *);

FractionObject *Fraction_Mul(FractionObject *, FractionObject *);

FractionObject *Fraction_Div(FractionObject *, FractionObject *);

int Fraction_IsZero(FractionObject *);

int Fraction_Eqv(FractionObject *self, Object *other);

FractionObject *Fraction_Neg(FractionObject *);

int Fraction_IsInteger(FractionObject *);

int Fraction_EQ(FractionObject *self, Object *other);

int Fraction_LT(FractionObject *self, Object *other);

#endif //R5RS_FRACTION_OBJECT_H
