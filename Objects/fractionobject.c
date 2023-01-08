#include <assert.h>
#include <math.h>
#include "../Include/typeobject.h"
#include "../Include/numberobject.h"
#include "../Include/pairobject.h"
#include "../Include/arrayobject.h"

CompareMethods fraction_compare = {
		.cmp_eqv = (int_binaryfunc) Fraction_Eqv,
};

NumberMethods fraction_as_number = {
		.nb_add=(binaryfunc) Fraction_Add,
		.nb_sub=(binaryfunc) Fraction_Sub,
		.nb_mul=(binaryfunc) Fraction_Mul,
		.nb_div=(binaryfunc) Fraction_Div,
		.nb_neg=(unaryfunc) Fraction_Neg,

		.nb_long=(unaryfunc) Long_FromFraction,
		.nb_fraction=Id,
		.nb_float=(unaryfunc) Float_FromFraction,
		.nb_complex=(unaryfunc) Complex_FromFraction,

		.nb_is_zero=(int_unaryfunc) Fraction_IsZero,
		.nb_is_exact=(int_unaryfunc) Always_True,
		.nb_is_inexact=(int_unaryfunc) Always_False,
		.nb_to_exact=(unaryfunc) Id,
		.nb_to_inexact=(unaryfunc) Float_FromFraction,

		.nb_is_integer=(int_unaryfunc) Fraction_IsInteger,
		.nb_is_rational=Always_True_Fast,
		.nb_is_real=Always_True_Fast,
		.nb_is_complex=Always_True_Fast,

		.nb_eq=(int_binaryfunc) Fraction_EQ,
		.nb_lt=(int_binaryfunc) Fraction_LT,

};
TypeObject Fraction_Type = {
		"fraction",
		.tp_basicsize=sizeof(FractionObject),
		.tp_itemsize=0,
		.tp_print=(print_proc) Fraction_Print,
		.tp_dealloc=(dealloc_proc) Fraction_Dealloc,
		.tp_as_number=&fraction_as_number,
		.tp_cmp=&fraction_compare,
		.tp_search=(search_proc) Fraction_Search,
};

FractionObject *Fraction_New(LongObject *numerator, LongObject *denominator, int normalize) {
	assert(!Long_IsZero(denominator));
	FractionObject *res = (FractionObject *) TypeGenericAlloc(&Fraction_Type, 0);

	if (normalize) {
		LongObject *g = Long_GCD(numerator, denominator);
		if (Long_Sign(denominator) < 0) {
			LongObject *tmp = NULL;
			MOVE_SET(tmp, g, Long_Neg(g));
		}
		res->_numerator = Long_Div(numerator, g);
		res->_denominator = Long_Div(denominator, g);
		DECREF(g);
	} else {
		res->_numerator = NEW_REF(numerator);
		res->_denominator = NEW_REF(denominator);
	}
	return res;
}

FractionObject *Fraction_FromLong(LongObject *op) {
	LongObject *one = Long_From_i64(1);
	FractionObject *res = Fraction_New(op, one, 0);
	DECREF(one);
	return res;
}

FractionObject *Fraction_FromFloat(FloatObject *op) {
	Object *p = Float_AsIntegerRatio(op);
	assert(IS_TYPE(CAR(p), Long_Type));
	assert(IS_TYPE(CDR(p), Long_Type));
	FractionObject *res = Fraction_New(AS_LONG(CAR(p)), AS_LONG(CDR(p)), 1);
	DECREF(p);
	return res;
}

FractionObject *Fraction_FromComplex(ComplexObject *op) {
	FloatObject *x = Float_FromComplex(op);
	FractionObject *res = Fraction_FromFloat(x);
	DECREF(x);
	return res;
}

void Fraction_Dealloc(FractionObject *self) {
	DECREF(self->_numerator);
	DECREF(self->_denominator);
}

void Fraction_Search(FractionObject *self, Object *target, ArrayObject *res) {
	APPEND_PARENT(target, self, self->_numerator);
	APPEND_PARENT(target, self, self->_denominator);

	SEARCH(self->_numerator, target, res);
	SEARCH(self->_denominator, target, res);
}

void Fraction_Print(FractionObject *self, FILE *out) {
	Long_Print(self->_numerator, out);
	fputc('/', out);
	Long_Print(self->_denominator, out);
}

// return value: New Reference
LongObject *Fraction_GetNumerator(FractionObject *self) {
	return NEW_REF(self->_numerator);
}

// return value: New Reference
LongObject *Fraction_GetDenominator(FractionObject *self) {
	return NEW_REF(self->_denominator);
}

FractionObject *Fraction_Add(FractionObject *a, FractionObject *b) {

	LongObject *na = a->_numerator;
	LongObject *da = a->_denominator;

	LongObject *nb = b->_numerator;
	LongObject *db = b->_denominator;

	LongObject *g = Long_GCD(da, db);

	if (Long_Cmp_i64(g, 1) == 0) {
		DECREF(g);
		LongObject *n1 = Long_Mul(na, db);
		LongObject *n2 = Long_Mul(da, nb);
		LongObject *n = Long_Add(n1, n2);
		DECREF(n1);
		DECREF(n2);
		LongObject *d = Long_Mul(da, db);
		FractionObject *res = Fraction_New(n, d, 0);
		DECREF(n);
		DECREF(d);
		return res;
	}
	LongObject *s = Long_Div(da, g);
	LongObject *t1 = Long_Div(db, g);
	LongObject *t2 = Long_Mul(na, t1);
	LongObject *t3 = Long_Mul(nb, s);
	LongObject *t = Long_Add(t2, t3);
	DECREF(t1);
	DECREF(t2);
	DECREF(t3);

	LongObject *g2 = Long_GCD(t, g);
	DECREF(g);

	FractionObject *res = 0;
	if (Long_Cmp_i64(g2, 1) == 0) {
		LongObject *d = Long_Mul(s, db);
		res = Fraction_New(t, d, 0);
		DECREF(d);
	} else {
		LongObject *n = Long_Div(t, g2);
		LongObject *d1 = Long_Div(db, g2);
		LongObject *d = Long_Mul(s, d1);
		res = Fraction_New(n, d, 0);
		DECREF(n);
		DECREF(d1);
		DECREF(d);
	}
	DECREF(s);
	DECREF(t);
	DECREF(g2);
	return res;
}

FractionObject *Fraction_Sub(FractionObject *a, FractionObject *b) {
	LongObject *na = a->_numerator;
	LongObject *da = a->_denominator;

	LongObject *nb = b->_numerator;
	LongObject *db = b->_denominator;

	LongObject *g = Long_GCD(da, db);

	if (Long_Cmp_i64(g, 1) == 0) {
		DECREF(g);
		LongObject *n1 = Long_Mul(na, db);
		LongObject *n2 = Long_Mul(da, nb);
		LongObject *n = Long_Sub(n1, n2);
		DECREF(n1);
		DECREF(n2);
		LongObject *d = Long_Mul(da, db);
		FractionObject *res = Fraction_New(n, d, 0);
		DECREF(n);
		DECREF(d);
		return res;
	}
	LongObject *s = Long_Div(da, g);

	LongObject *t1 = Long_Div(db, g);
	LongObject *t2 = Long_Mul(na, t1);
	LongObject *t3 = Long_Mul(nb, s);
	LongObject *t = Long_Sub(t2, t3);
	DECREF(t1);
	DECREF(t2);
	DECREF(t3);

	LongObject *g2 = Long_GCD(t, g);
	DECREF(g);

	FractionObject *res = 0;
	if (Long_Cmp_i64(g2, 1) == 0) {
		LongObject *d = Long_Mul(s, db);
		res = Fraction_New(t, d, 0);
		DECREF(d);
	} else {
		LongObject *n = Long_Div(t, g2);
		LongObject *d1 = Long_Div(db, g2);
		LongObject *d = Long_Mul(s, d1);
		res = Fraction_New(n, d, 0);
		DECREF(n);
		DECREF(d1);
		DECREF(d);
	}
	DECREF(s);
	DECREF(t);
	DECREF(g2);
	return res;

}

FractionObject *Fraction_Mul(FractionObject *a, FractionObject *b) {
	LongObject *na = Fraction_GetNumerator(a);
	LongObject *da = Fraction_GetDenominator(a);
	LongObject *nb = Fraction_GetNumerator(b);
	LongObject *db = Fraction_GetDenominator(b);
	LongObject *g1 = Long_GCD(na, db);
	LongObject *tmp;
	if (Long_Cmp_i64(g1, 1) > 0) {
		MOVE_SET(tmp, na, Long_Div(na, g1));
		MOVE_SET(tmp, db, Long_Div(db, g1));
	}
	LongObject *g2 = Long_GCD(nb, da);
	if (Long_Cmp_i64(g2, 1) > 0) {
		MOVE_SET(tmp, nb, Long_Div(nb, g2));
		MOVE_SET(tmp, da, Long_Div(da, g2));
	}
	LongObject *n = Long_Mul(na, nb);
	LongObject *d = Long_Mul(db, da);
	FractionObject *res = Fraction_New(n, d, 0);
	DECREF(d);
	DECREF(n);
	DECREF(g2);
	DECREF(g1);
	DECREF(db);
	DECREF(nb);
	DECREF(da);
	DECREF(na);
	return res;
}

FractionObject *Fraction_Div(FractionObject *a, FractionObject *b) {
	LongObject *na = Fraction_GetNumerator(a);
	LongObject *da = Fraction_GetDenominator(a);
	LongObject *nb = Fraction_GetNumerator(b);
	LongObject *db = Fraction_GetDenominator(b);
	LongObject *g1 = Long_GCD(na, nb);
	LongObject *tmp;

	if (Long_Cmp_i64(g1, 1) > 0) {
		MOVE_SET(tmp, na, Long_Div(na, g1));
		MOVE_SET(tmp, nb, Long_Div(nb, g1));
	}
	LongObject *g2 = Long_GCD(db, da);
	if (Long_Cmp_i64(g2, 1) > 0) {
		MOVE_SET(tmp, da, Long_Div(da, g2));
		MOVE_SET(tmp, db, Long_Div(db, g2));
	}
	LongObject *n = Long_Mul(na, db);
	LongObject *d = Long_Mul(nb, da);
	if (Long_Sign(d) < 0) {
		MOVE_SET(tmp, n, Long_Neg(n));
		MOVE_SET(tmp, d, Long_Neg(d));
	}
	FractionObject *res = Fraction_New(n, d, 0);
	DECREF(n);
	DECREF(d);
	DECREF(g2);
	DECREF(g1);
	DECREF(na);
	DECREF(da);
	DECREF(nb);
	DECREF(db);
	return res;
}

int Fraction_IsZero(FractionObject *self) {
	return Long_IsZero(self->_numerator);
}

int Fraction_Eqv(FractionObject *self, Object *other) {
	if (IS_TYPE(other, Fraction_Type)) {
		return EQV(self->_numerator, AS_FRAC(other)->_numerator)
			   && EQV(self->_denominator, AS_FRAC(other)->_denominator);
	}
	if (IS_TYPE(other, Long_Type)) {
		return Fraction_IsInteger(self) && EQV(self->_numerator, AS_LONG(other));
	}
	return 0;
}

FractionObject *Fraction_Neg(FractionObject *self) {
	if (Long_IsZero(self->_numerator)) {
		return NEW_REF(self);
	}
	LongObject *n = Long_Neg(self->_numerator);
	FractionObject *res = Fraction_New(n, self->_denominator, 0);
	DECREF(n);
	return res;
}

int Fraction_IsInteger(FractionObject *self) {
	return Long_Cmp_i64(self->_denominator, 1) == 0;
}

int Fraction_EQ(FractionObject *self, Object *other) {
	assert(IS_NUMBER(other));
	if (IS_INEXACT(other)) return NUMBER_METHODS(other)->nb_eq(other, AS_OBJECT(self));
	return Fraction_Eqv(self, other);
}

int Fraction_LT(FractionObject *self, Object *other) {
	assert(IS_REAL(other));
	FloatObject *x = Float_FromFraction(self);
	int res = Float_LT(x, other);
	DECREF(x);
	return res;
}
