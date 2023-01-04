#ifndef R5RS_LONG_OBJECT_H
#define R5RS_LONG_OBJECT_H

#include "object.h"

struct long_object {
	Object ob_base;
	i64 ob_val;
};
extern TypeObject Long_Type;

void Long_SmallIntsInitialize();

LongObject *Long_From_i64(i64);

LongObject *Long_FromFloat(FloatObject *);

LongObject *Long_FromDouble(double);

LongObject *Long_FromFraction(FractionObject *);

LongObject *Long_FromComplex(ComplexObject *);
//void Long_Dealloc(LongObject *);

void Long_Print(LongObject *, FILE *);

LongObject *Long_Add(LongObject *, LongObject *);

LongObject *Long_Sub(LongObject *, LongObject *);

LongObject *Long_Mul(LongObject *, LongObject *);

LongObject *Long_MulByI64(LongObject *, i64);

LongObject *Long_Div(LongObject *, LongObject *);

LongObject *Long_Remainder(LongObject *, LongObject *);

LongObject *Long_Pow(LongObject *, LongObject *);

LongObject *Long_Neg(LongObject *);

LongObject *Long_LeftShift(LongObject *, LongObject *);

LongObject *Long_RightShift(LongObject *, LongObject *);

LongObject *Long_GCD(LongObject *, LongObject *);

LongObject *Long_Abs(LongObject *);

int Long_IsZero(LongObject *);

int Long_IsEven(LongObject *);

int Long_IsOdd(LongObject *);

int Long_Sign(LongObject *);


int Long_Eqv(LongObject *self, Object *other);

int Long_EQ(LongObject *, Object *);

int Long_LT(LongObject *, Object *);

int Long_Cmp_i64(LongObject *, i64);

#endif //R5RS_LONG_OBJECT_H
