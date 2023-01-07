#ifndef R5RS_NUMBER_OBJECT_H
#define R5RS_NUMBER_OBJECT_H


#include "object.h"
#include "floatobject.h"
#include "complexobject.h"
#include "longobject.h"
#include "fractionobject.h"


Object *IsNumber(Object *);

Object *IsComplex(Object *);

Object *IsReal(Object *);

Object *IsRational(Object *);

Object *IsInteger(Object *);

Object *IsZero(Object *);

Object *IsExact(Object *);

Object *IsInexact(Object *);

Object *ToExact(Object *);

Object *ToInexact(Object *);

Object *Id(Object *);

Object *Always_True(Object *);

Object *Always_False(Object *);

int IsNumberFast(Object *);

int IsComplexFast(Object *);

int IsRealFast(Object *);

int IsRationalFast(Object *);

int IsIntegerFast(Object *);

int IsZeroFast(Object *);

int IsExactFast(Object *);

int IsInexactFast(Object *);


int Always_True_Fast(Object *);

int Always_False_Fast(Object *);

Object *Number_Add(Object *, Object *);

Object *Number_Sub(Object *, Object *);


Object *Number_Mul(Object *, Object *);


Object *Number_Div(Object *, Object *);

Object *Number_Pow(Object *, Object *);

Object *Number_Neg(Object *);

int Number_Eqv(Object *, Object *);

int Number_EQ(Object *x, Object *y);

int Number_LT(Object *x, Object *y);

int Number_LE(Object *x, Object *y);

int Number_GT(Object *x, Object *y);

int Number_GE(Object *x, Object *y);

Object *ExactToInexact(Object *);

Object *InexactToExact(Object *);

#define NUMBER_METHODS(obj) CAST(NumberMethods*, TYPE(obj)->tp_as_number)
#define IS_NUMBER(obj) IsNumberFast(AS_OBJECT(obj))
#define IS_ZERO(obj) IsZeroFast(AS_OBJECT(obj))
#define IS_INTEGER(obj) IsIntegerFast(AS_OBJECT(obj))
#define IS_REAL(obj) IsRealFast(AS_OBJECT(obj))
#define IS_EXACT(obj) IsExactFast(AS_OBJECT(obj))
#define IS_INEXACT(obj) IsInexactFast(AS_OBJECT(obj))
#define TO_EXACT(num) (NUMBER_METHODS(num)->nb_to_exact(AS_OBJECT(num)))
#define TO_INEXACT(num) (NUMBER_METHODS(num)->nb_to_inexact(AS_OBJECT(num)))
#endif //R5RS_NUMBER_OBJECT_H




