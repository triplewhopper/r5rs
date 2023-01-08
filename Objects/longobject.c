#include <assert.h>
#include <math.h>
#include <inttypes.h>
#include "../Include/typeobject.h"
#include "../Include/numberobject.h"
#include "../Include/booleanobject.h"

CompareMethods long_compare = {
		.cmp_eqv = (int_binaryfunc) Long_Eqv,
};
NumberMethods long_as_number = {
		.nb_add=(binaryfunc) Long_Add,
		.nb_sub=(binaryfunc) Long_Sub,
		.nb_mul=(binaryfunc) Long_Mul,
		.nb_div=(binaryfunc) Long_Div,
		.nb_neg=(unaryfunc) Long_Neg,

		.nb_long=Id,
		.nb_fraction=(unaryfunc) Fraction_FromLong,
		.nb_float=(unaryfunc) Float_FromLong,
		.nb_complex=(unaryfunc) Complex_FromLong,

		.nb_is_zero=(int_unaryfunc) Long_IsZero,
		.nb_is_exact=Always_True_Fast,
		.nb_is_inexact=Always_False_Fast,
		.nb_to_exact=Id,
		.nb_to_inexact=(unaryfunc) Float_FromLong,

		.nb_is_integer=Always_True_Fast,
		.nb_is_rational=Always_True_Fast,
		.nb_is_real=Always_True_Fast,
		.nb_is_complex=Always_True_Fast,

		.nb_lshift=(binaryfunc) Long_LeftShift,
		.nb_rshift=(binaryfunc) Long_RightShift,

		.nb_eq=(int_binaryfunc) Long_EQ,
		.nb_lt=(int_binaryfunc) Long_LT,
};
TypeObject Long_Type = {
		"long",
		.tp_basicsize=sizeof(LongObject),
		.tp_itemsize=0,
		.tp_print=(print_proc) Long_Print,
//		.tp_dealloc=(dealloc_proc) Long_Dealloc,
		.tp_as_number = &long_as_number,
		.tp_cmp=&long_compare
};

static LongObject *Long_Alloc() {
	return (LongObject *) TypeGenericAlloc(&Long_Type, 0);
}

static LongObject *Long_New(size_t size);

#ifdef SMALL_INT_OPTIMIZATION
static LongObject small_ints[261]; // -5..255

static LongObject *get_small_ints(i64 val) {
	return NEW_REF(&small_ints[val + 5]);
}
#endif

void smallints_init() {
#ifdef SMALL_INT_OPTIMIZATION
	static int small_ints_init_flag = 0;
	if (!small_ints_init_flag) {
		small_ints_init_flag = 1;
		for (i64 i = -5; i < 256; ++i) {
			small_ints[i + 5] = (LongObject) {1, &Long_Type, i};
		}
	}
#endif
}


LongObject *Long_From_i64(i64 val) {
#ifdef SMALL_INT_OPTIMIZATION
	if (-5 <= val && val <= 255) {
		return get_small_ints(val);
	}
#endif
	LongObject *res = Long_Alloc();
	res->ob_val = val;
	return res;
}

LongObject *Long_FromFloat(FloatObject *x) {
	return Long_FromDouble(Float_GetVal(x));
}

LongObject *Long_FromDouble(double x) {
	if (isfinite(x)) {
		return Long_From_i64((i64) trunc(x));
	} else if (isnan(x)) {
		fprintf(stderr, "in Long_FromFloat(): NaN cannot be converted to an integer.\n");
	} else if (isinf(x)) {
		fprintf(stderr, "in Long_FromFloat(): Inf cannot be converted to an integer.\n");
	} else {
		fprintf(stderr, "in Long_FromFloat(): Not Implemented yet.\n");
	}
	exit(EXIT_FAILURE);
}

LongObject *Long_FromFraction(FractionObject *op) {
	return Long_Div(op->_numerator, op->_denominator);
}

LongObject *Long_FromComplex(ComplexObject *op) {
	if (op->im == 0.) {
		return Long_FromDouble(op->re);
	}
	fprintf(stderr, "imagine part is not zero.\n");
	exit(EXIT_FAILURE);
}

void Long_Print(LongObject *self, FILE *out) {
	fprintf(out, "%" PRId64, self->ob_val);
}

static i64 Long_GetVal(LongObject *self) {
	return self->ob_val;
}

LongObject *Long_Add(LongObject *self, LongObject *other) {
	return Long_From_i64(Long_GetVal(self) + Long_GetVal(other));
}

LongObject *Long_Sub(LongObject *self, LongObject *other) {
	return Long_From_i64(Long_GetVal(self) - Long_GetVal(other));
}

LongObject *Long_Mul(LongObject *self, LongObject *other) {
	return Long_From_i64(Long_GetVal(self) * Long_GetVal(other));
}

LongObject *Long_MulByI64(LongObject *self, i64 other) {
	return Long_From_i64(Long_GetVal(self) * other);
}

LongObject *Long_Div(LongObject *self, LongObject *other) {
	if (!Long_IsZero(other)) {
		return Long_From_i64(Long_GetVal(self) / Long_GetVal(other));
	}
	fprintf(stderr, "integer division by zero\n");
	exit(EXIT_FAILURE);
}

LongObject *Long_Remainder(LongObject *self, LongObject *other) {
	if (!Long_IsZero(other)) {
		return Long_From_i64(Long_GetVal(self) % Long_GetVal(other));
	}
	fprintf(stderr, "integer modulo by zero\n");
	exit(EXIT_FAILURE);
}

// *q and *r must not be strong reference when passed.
static void Long_DivRem(LongObject *self, LongObject *divisor, LongObject **q, LongObject **r) {
	*q = Long_From_i64(self->ob_val / divisor->ob_val);
	*r = Long_From_i64(self->ob_val % divisor->ob_val);
}

LongObject *Long_Pow(LongObject *self, LongObject *other) {
	assert(Long_Sign(other) >= 0);
	INCREF(other);
	INCREF(self);
	LongObject *ans = Long_From_i64(1);
	LongObject *two = Long_From_i64(2);
	LongObject *tmp = NULL;
	while (1) {
		if (Long_IsZero(other)) {
			if (Long_IsOdd(other)) {
				MOVE_SET(tmp, ans, Long_Mul(ans, self));
			}
			MOVE_SET(tmp, self, Long_Mul(self, self));
			MOVE_SET(tmp, other, Long_Div(other, two));
		} else {
			break;
		}
	}
	DECREF(two);
	DECREF(self);
	DECREF(other);
	return ans;
}

LongObject *Long_Neg(LongObject *self) {
	return Long_From_i64(-Long_GetVal(self));
}

// steals self, other
LongObject *Long_LeftShift(LongObject *self, LongObject *other) {
	if (Long_Sign(other) > 0) {
		LongObject *res = Long_From_i64(self->ob_val << other->ob_val);
		DECREF(self);
		DECREF(other);
		return res;
	}
	fprintf(stderr, "Left shifting with a negative shift count.\n");
	exit(EXIT_FAILURE);
}

// steals self, other
LongObject *Long_RightShift(LongObject *self, LongObject *other) {
	if (Long_Sign(other) > 0) {
		LongObject *res = Long_From_i64(self->ob_val >> other->ob_val);
		DECREF(self);
		DECREF(other);
		return res;
	}
	fprintf(stderr, "Right shifting with a negative shift count.\n");
	exit(EXIT_FAILURE);
}

LongObject *Long_GCD(LongObject *x, LongObject *y) {
	INCREF(x), INCREF(y);
	LongObject *tmp = NULL;
	while (!Long_IsZero(y)) {
		tmp = Long_Remainder(x, y);
		DECREF(x);
		x = y;
		y = tmp;
	}
	DECREF(y);
	if(Long_Sign(x)<0)
		MOVE_SET(tmp, x, Long_Neg(x));
	return x;
}

LongObject *Long_Abs(LongObject *self) {
	if (Long_GetVal(self) > 0) {
		return NEW_REF(self);
	}
	return Long_From_i64(-Long_GetVal(self));
}


int Long_IsZero(LongObject *self) {
	return Long_Sign(self) == 0;
}

int Long_IsEven(LongObject *self) {
	return Long_GetVal(self) % 2 == 0;
}

int Long_IsOdd(LongObject *self) {
	return Long_GetVal(self) % 2 != 0;
}

int Long_Sign(LongObject *self) {
	if (Long_GetVal(self) > 0) {
		return 1;
	} else if (Long_GetVal(self) < 0) {
		return -1;
	}
	return 0;
}

int Long_Eqv(LongObject *self, Object *other) {
	if (IS_TYPE(other, Long_Type)) {
		return self->ob_val == AS_LONG(other)->ob_val;
	}
	if (IS_TYPE(other, Fraction_Type)) {
		return Fraction_IsInteger(AS_FRAC(other)) && EQV(self, AS_FRAC(other)->_numerator);
	}
	return 0;
}

int Long_EQ(LongObject *self, Object *other) {
	if (Long_Eqv(self, other)) return 1;
	assert(IS_NUMBER(other));
	if (!IS_INTEGER(other)) return 0;
	LongObject *y = AS_LONG(NUMBER_METHODS(other)->nb_long(other));
	int res = self->ob_val == y->ob_val;
	DECREF(y);
	return res;
}

int Long_LT(LongObject *self, Object *other) {
	assert(IS_REAL(other));
	if (IS_INTEGER(other)) {
		LongObject *y = AS_LONG(NUMBER_METHODS(other)->nb_long(other));
		int res = self->ob_val < y->ob_val;
		DECREF(y);
		return res;
	} else {
		return !Long_EQ(self, other) && !NUMBER_METHODS(other)->nb_lt(other, AS_OBJECT(self));
	}
}

int Long_Cmp_i64(LongObject *self, i64 other) {
	if (Long_GetVal(self) > other) {
		return 1;
	} else if (Long_GetVal(self) < other) {
		return -1;
	}
	return 0;
}


