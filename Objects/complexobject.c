#include <stdio.h>
#include <math.h>
#include "../Include/typeobject.h"
#include "../Include/numberobject.h"

CompareMethods complex_compare = {
		.cmp_eqv = (int_binaryfunc) Complex_Eqv,
};

NumberMethods complex_as_number = {
		.nb_add=(binaryfunc) Complex_Add,
		.nb_sub=(binaryfunc) Complex_Sub,
		.nb_mul=(binaryfunc) Complex_Mul,
		.nb_div=(binaryfunc) Complex_Div,
		.nb_neg=(unaryfunc) Complex_Neg,

		.nb_long=(unaryfunc) Long_FromComplex,
		.nb_fraction=(unaryfunc) Fraction_FromComplex,
		.nb_float=(unaryfunc) Float_FromComplex,
		.nb_complex=(unaryfunc) Id,

		.nb_is_zero=(int_unaryfunc) Complex_IsZero,
		.nb_is_exact=(int_unaryfunc) Always_False_Fast,
		.nb_is_inexact=(int_unaryfunc) Always_True_Fast,
		.nb_to_exact=(unaryfunc) Complex_ToExact,
		.nb_to_inexact=(unaryfunc) Id,

		.nb_is_integer=(int_unaryfunc) Complex_IsInteger,
		.nb_is_rational=(int_unaryfunc) Complex_IsRational,
		.nb_is_real=(int_unaryfunc) Complex_IsRational,
		.nb_is_complex=Always_True_Fast,

		.nb_eq=(int_binaryfunc) Complex_EQ,
};

TypeObject Complex_Type = {
		.tp_name="complex",
		.tp_basicsize=sizeof(ComplexObject),
		.tp_itemsize=0,
		.tp_print=(print_proc) Complex_Print,
		.tp_as_number=&complex_as_number,
		.tp_cmp = &complex_compare
};

ComplexObject *Complex_New(double re, double im) {
	ComplexObject *res = (ComplexObject *) TypeGenericAlloc(&Complex_Type, 0);
	res->re = re;
	res->im = im;
	return res;
}

ComplexObject *Complex_FromFloat(FloatObject *op) {
	return Complex_New(Float_GetVal(op), 0.);
}

ComplexObject *Complex_FromFraction(FractionObject *op) {
	FloatObject *x = Float_FromFraction(op);
	ComplexObject *res = Complex_FromFloat(x);
	DECREF(x);
	return res;
}

ComplexObject *Complex_FromLong(LongObject *op) {
	FloatObject *x = Float_FromLong(op);
	ComplexObject *res = Complex_FromFloat(x);
	DECREF(x);
	return res;
}

ComplexObject *Complex_FromExpForm(Object *r, Object *theta) {
	if (IS_REAL(r) && IS_REAL(theta)) {
		Object *rr = NUMBER_METHODS(r)->nb_float(r);
		Object *ttheta = NUMBER_METHODS(theta)->nb_float(theta);
		assert(IS_TYPE(rr, Float_Type));
		assert(IS_TYPE(ttheta, Float_Type));
		ComplexObject *res = Complex_FromExpFormCDouble(
				Float_GetVal(AS_FLOAT(rr)),
				Float_GetVal(AS_FLOAT(ttheta)));
		DECREF(rr);
		DECREF(ttheta);
		return res;
	}
	if (!IS_REAL(r)) fprintf(stderr, "argument r: expect real, got %s\n", TYPE(r)->tp_name);
	if (!IS_REAL(theta)) fprintf(stderr, "argument theta: expect real, got %s\n", TYPE(theta)->tp_name);
	exit(EXIT_FAILURE);
}

ComplexObject *Complex_FromExpFormCDouble(double r, double theta) {
	return Complex_New(r * cos(theta), r * sin(theta));
}

void Complex_Print(ComplexObject *self, FILE *out) {
	fprintf(out, "%g%+gi", self->re, self->im);
}

ComplexObject *Complex_Add(ComplexObject *a, ComplexObject *b) {
	return Complex_New(a->re + b->re, a->im + b->im);
}

ComplexObject *Complex_Sub(ComplexObject *a, ComplexObject *b) {
	return Complex_New(a->re - b->re, a->im - b->im);
}

ComplexObject *Complex_Mul(ComplexObject *a, ComplexObject *b) {
	return Complex_New(
			a->re * b->re - a->im * b->im,
			a->re * b->im + a->im * b->re);
}

ComplexObject *Complex_Div(ComplexObject *a, ComplexObject *b) {
	return Complex_New(
			a->re * b->re - a->im * b->im,
			a->re * b->im + a->im * b->re);
}

ComplexObject *Complex_Neg(ComplexObject *self) {
	return Complex_New(-self->re, -self->im);
}

ComplexObject *Complex_Conjugate(ComplexObject *self) {
	return Complex_New(self->re, -self->im);
}

FloatObject *Complex_Abs(ComplexObject *self) {
	return Float_New(sqrt(self->re * self->re + self->im * self->im));
}

int Complex_IsZero(ComplexObject *self) {
	return self->re == 0. && self->im == 0.;
}

Object *Complex_ToExact(ComplexObject *self) {
	if (self->im == 0.) {
		FloatObject *re = Float_New(self->re);
		Object *res = Float_ToExact(re);
		DECREF(re);
		return res;
	}
	fprintf(stderr, "cannot convert a complex with non-zero imag-part to exact.\n");
	exit(EXIT_FAILURE);
}

int Complex_IsInteger(ComplexObject *self) {
	return trunc(self->re) == self->re && self->im == 0.;
}

int Complex_IsRational(ComplexObject *self) {
	return isfinite(self->re) && self->im == 0.;
}

int Complex_Eqv(ComplexObject *self, Object *z2) {
	if (IS_TYPE(z2, Complex_Type)) {
		return self->re == AS_COMPLEX(z2)->re && self->im == AS_COMPLEX(z2)->im;
	}
	if (IS_TYPE(z2, Float_Type)) {
		if (self->im == 0.) {
			return self->re == AS_FLOAT(z2)->ob_fval;
		}
	}
	return 0;
}

int Complex_EQ(ComplexObject *self, Object *other) {
	if (Complex_Eqv(self, other)) return 1;
	assert(IS_NUMBER(other));
	ComplexObject *z = AS_COMPLEX(NUMBER_METHODS(other)->nb_complex(other));
	int res = self->re == z->re && self->im == z->im;
	DECREF(z);
	return res;
}


