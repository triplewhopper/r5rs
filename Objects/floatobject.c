#include <math.h>
#include <limits.h>
#include <assert.h>
#include "../Include/typeobject.h"
#include "../Include/numberobject.h"
#include "../Include/pairobject.h"

CompareMethods float_compare = {
		.cmp_eqv = (int_binaryfunc) Float_Eqv,
};

NumberMethods float_as_number = {
		.nb_add=(binaryfunc) Float_Add,
		.nb_sub=(binaryfunc) Float_Sub,
		.nb_mul=(binaryfunc) Float_Mul,
		.nb_div=(binaryfunc) Float_Div,
		.nb_neg=(unaryfunc) Float_Neg,

		.nb_long=(unaryfunc) Long_FromFloat,
		.nb_fraction=(unaryfunc) Fraction_FromFloat,
		.nb_float=Id,
		.nb_complex=(unaryfunc) Complex_FromFloat,

		.nb_is_zero=(int_unaryfunc) Fraction_IsZero,
		.nb_is_exact=Always_False_Fast,
		.nb_is_inexact=Always_True_Fast,
		.nb_to_exact=(unaryfunc) Float_ToExact,
		.nb_to_inexact=Id,

		.nb_is_integer=(int_unaryfunc) Float_IsInteger,
		.nb_is_rational=(int_unaryfunc) Float_IsRational,
		.nb_is_real=Always_True_Fast,
		.nb_is_complex=Always_True_Fast,

		.nb_eq=(int_binaryfunc) Float_EQ,
		.nb_lt=(int_binaryfunc) Float_LT,
};

TypeObject Float_Type = {
		"float",
		.tp_basicsize=sizeof(FloatObject),
		.tp_itemsize=0,
		.tp_print=(print_proc) Float_Print,
		.tp_as_number=&float_as_number,
		.tp_cmp = &float_compare
};

static FloatObject *Float_Alloc() {
	return (FloatObject *) TypeGenericAlloc(&Float_Type, 0);
}

FloatObject *Float_New(double val) {
	FloatObject *res = Float_Alloc();
	assert(isfinite(val));
	res->ob_fval = val;
	return res;
}

FloatObject *Float_FromFraction(FractionObject *op) {
	FloatObject *n = Float_FromLong(op->_numerator);
	FloatObject *d = Float_FromLong(op->_denominator);
	FloatObject *quotient = Float_Div(n, d);
	DECREF(n);
	DECREF(d);
	return quotient;
}

FloatObject *Float_FromLong(LongObject *op) {
	return Float_New((double) op->ob_val);
}

FloatObject *Float_FromComplex(ComplexObject *op) {
	if (op->im == 0.) {
		return Float_New(op->re);
	}
	fprintf(stderr, "imag-part is not zero.\n");
	exit(EXIT_FAILURE);
}

static Object *as_integer_ratio(double x) {
	if (isinf(x)) {
		fprintf(stderr, "cannot convert Infinity to integer ratio\n");
		exit(EXIT_FAILURE);
	}
	if (isnan(x)) {
		fprintf(stderr, "cannot convert NaN to integer ratio\n");
		exit(EXIT_FAILURE);
	}
	int exponent = 0;
	double float_part = frexp(x, &exponent);
	for (size_t i = 0; i < 64 && trunc(float_part) != float_part; ++i) {
		//TODO: has to be fixed when implementing arbitrary precision number
		float_part *= 2.0;
		exponent--;
	}
	LongObject *numerator = Long_FromDouble(float_part);
	LongObject *denominator = Long_From_i64(1);
	LongObject *e = Long_From_i64(abs(exponent));
	if (exponent > 0) {
		LongObject *tmp = NULL;
		MOVE_SET(tmp, numerator, Long_LeftShift(numerator, e));
	} else {
		LongObject *tmp = NULL;
		MOVE_SET(tmp, denominator, Long_RightShift(denominator, e));
	}
	Object *res = CONS(numerator, denominator);
	DECREF(numerator);
	DECREF(denominator);
	DECREF(e);
	return res;
}

Object *Float_AsIntegerRatio(FloatObject *self) {
	return as_integer_ratio(Float_GetVal(self));
}
//void Float_Dealloc(FloatObject *op) {
//	free(op);
//}

void Float_Print(FloatObject *self, FILE *out) {
	static char buf[100];
	sprintf(buf, "%.15g", self->ob_fval);
	fprintf(out, "%s", buf);
	int flag = 0;
	for (char *p = buf; *p; ++p) {
		if (*p == '.' || *p == 'e' || *p == 'E') {
			flag = 1;
			break;
		}
	}
	if (flag == 0) {
		fprintf(out, ".0");
	}
}

double Float_GetVal(FloatObject *self) {
	return self->ob_fval;
}

FloatObject *Float_Add(FloatObject *self, FloatObject *other) {
	return Float_New(self->ob_fval + other->ob_fval);
}

FloatObject *Float_Sub(FloatObject *self, FloatObject *other) {
	return Float_New(self->ob_fval - other->ob_fval);
}

FloatObject *Float_Mul(FloatObject *self, FloatObject *other) {
	return Float_New(self->ob_fval * other->ob_fval);
}

FloatObject *Float_MulByDouble(FloatObject *self, double other) {
	return Float_New(self->ob_fval * other);
}

FloatObject *Float_Div(FloatObject *self, FloatObject *other) {
	return Float_New(self->ob_fval / other->ob_fval);
}

FloatObject *Float_Abs(FloatObject *self) {
	if (Float_GetVal(self) > 0.) {
		return NEW_REF(self);
	}
	return Float_Neg(self);
}

FloatObject *Float_Neg(FloatObject *self) {
	return Float_New(-Float_GetVal(self));
}

Object *Float_ToExact(FloatObject *self) {
	if (Float_IsInteger(self)) {
		return AS_OBJECT(Long_FromFloat(self));
	} else {
		return AS_OBJECT(Fraction_FromFloat(self));
	}
}


int Float_Eqv(FloatObject *self, Object *other) {
	if (IS_TYPE(other, Float_Type)) {
		return self->ob_fval == AS_FLOAT(other)->ob_fval;
	}
	if (IS_TYPE(other, Complex_Type)) {
		return self->ob_fval == AS_COMPLEX(other)->re && AS_COMPLEX(other)->im == 0.;
	}
	return 0;
}

int Float_IsZero(FloatObject *self) {
	return self->ob_fval == 0.;
}

int Float_IsInteger(FloatObject *self) {
	double x = self->ob_fval;
	return isfinite(x) && trunc(x) == x;
}

int Float_IsRational(FloatObject *self) {
	return isfinite(self->ob_fval);
}

int Float_IsNaN(FloatObject *self) {
	return isnan(self->ob_fval);
}

int Float_IsINF(FloatObject *self) {
	return isinf(self->ob_fval);
}

int Float_IsFinite(FloatObject *self) {
	return isfinite(self->ob_fval);
}

int Float_LT(FloatObject *self, Object *other) {
	assert(IS_REAL(other));
	FloatObject *x = AS_FLOAT(NUMBER_METHODS(other)->nb_float(other));
	int res = self->ob_fval < x->ob_fval;
	DECREF(x);
	return res;
}

int Float_EQ(FloatObject *self, Object *other) {
	if (Float_Eqv(self, other)) return 1;
	assert(IS_NUMBER(other));
	FloatObject *x = AS_FLOAT(NUMBER_METHODS(other)->nb_float(other));
	int res = self->ob_fval == x->ob_fval;
	DECREF(x);
	return res;
}

