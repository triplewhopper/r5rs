#include "../Include/object.h"
#include "../Include/typeobject.h"
#include "../Include/numberobject.h"
#include "../Include/booleanobject.h"
#include "../library/runtime.h"
#include <assert.h>
#include <math.h>

#define MAKE_IS_FUNCS(name) \
Object *name(Object *op) { \
    if(name##Fast(op)) RETURN_TRUE; \
    RETURN_FALSE; \
}

#define MAKE_INT_FUNC(name, method) \
int name(Object *op) { \
    if (TYPE(op)->tp_as_number) { \
        if (TYPE(op)->tp_as_number->nb_is_##method) { \
            return TYPE(op)->tp_as_number->nb_is_##method(op); \
        } \
    } \
    return 0;\
}

#define MAKE_NUMBER_FUNC(name) \
Object *name(Object *op) { \
    if(name##Fast(op)) RETURN_TRUE; \
    RETURN_FALSE; \
}

#define MAKE_INT_NUMBER_FUNC(name, method) \
int name(Object *op) { \
    if (TYPE(op)->tp_as_number) { \
        if (TYPE(op)->tp_as_number->nb_is_##method) { \
            return TYPE(op)->tp_as_number->nb_is_##method(op); \
        } \
    } \
    fprintf(stderr, "in function %s, wrong type argument in position 1: ", __FUNCTION__); \
    TYPE(op)->tp_print(op, stderr); \
    fprintf(stderr, "\n"); \
    exit(EXIT_FAILURE); \
}

MAKE_IS_FUNCS(IsNumber)

MAKE_IS_FUNCS(IsComplex)

MAKE_IS_FUNCS(IsReal)

MAKE_IS_FUNCS(IsRational)

MAKE_IS_FUNCS(IsInteger)

MAKE_IS_FUNCS(IsZero)

MAKE_NUMBER_FUNC(IsExact)

MAKE_NUMBER_FUNC(IsInexact)

Object *Id(Object *op) {
	return NewRef(op);
}

Object *Always_True(Object *op) {
	RETURN_TRUE;
}

Object *Always_False(Object *op) {
	RETURN_FALSE;
}

int Always_True_Fast(Object *op) {
	return 1;
}

int Always_False_Fast(Object *op) {
	return 0;
}

int IsNumberFast(Object *op) {
	return IsComplexFast(op);
}

MAKE_INT_FUNC(IsComplexFast, complex)

MAKE_INT_FUNC(IsRealFast, real)

MAKE_INT_FUNC(IsRationalFast, rational)

MAKE_INT_FUNC(IsIntegerFast, integer)

MAKE_INT_FUNC(IsZeroFast, zero)

MAKE_INT_NUMBER_FUNC(IsExactFast, exact)

MAKE_INT_NUMBER_FUNC(IsInexactFast, inexact)


enum BinaryOpCode {
	ADD, SUB, MUL, DIV
};
#define call_unaryfunc(f, x) TYPE(x)->tp_as_number->f(x)
#define call_binaryfunc(f, x, y) TYPE(x)->tp_as_number->f(x, y)

static Object *exec_binaryfunc(Object *x, Object *y, enum BinaryOpCode opcode) {
	switch (opcode) {
		case ADD:
			return call_binaryfunc(nb_add, x, y);
		case SUB:
			return call_binaryfunc(nb_sub, x, y);
		case MUL:
			return call_binaryfunc(nb_mul, x, y);
		case DIV:
			return call_binaryfunc(nb_div, x, y);
		default:
			fprintf(stderr, "Not Implemented.\n");
			exit(EXIT_FAILURE);
	}
}

static Object *Number_BinaryOp(Object *x, Object *y, enum BinaryOpCode opcode) {
	if (IS_NUMBER(x)) {
		if (IS_NUMBER(y)) {
			INCREF(x), INCREF(y);
			Object *res = NULL, *tmp = NULL;
			if (IS_EXACT(x) && IS_EXACT(y)) {
				if (IsIntegerFast(x) && IsIntegerFast(y)) {
					MOVE_SET(tmp, x, call_unaryfunc(nb_long, x));
					MOVE_SET(tmp, y, call_unaryfunc(nb_long, y));
				} else {
					MOVE_SET(tmp, x, call_unaryfunc(nb_fraction, x));
					MOVE_SET(tmp, y, call_unaryfunc(nb_fraction, y));
				}
				res = exec_binaryfunc(x, y, opcode);
				if (IS_INTEGER(res)) {
					MOVE_SET(tmp, res, call_unaryfunc(nb_long, res));
				}
			} else {
				if (IS_REAL(x) && IS_REAL(y)) {
					MOVE_SET(tmp, x, call_unaryfunc(nb_float, x));
					MOVE_SET(tmp, y, call_unaryfunc(nb_float, y));
				} else {
					MOVE_SET(tmp, x, call_unaryfunc(nb_complex, x));
					MOVE_SET(tmp, y, call_unaryfunc(nb_complex, y));
				}
				res = exec_binaryfunc(x, y, opcode);
			}
			DECREF(x), DECREF(y);
			return AS_OBJECT(res);
		} else {
//			char opcode_name = "+-*/"[opcode];
			fprintf(stderr, "wrong type argument in position 2: ");
			PRINT(y, stderr);
			fprintf(stderr, "\n");
		}
	} else {
		char opcode_name = "+-*/"[opcode];
		fprintf(stderr, "wrong type argument in position 1: ");
		TYPE(x)->tp_print(x, stderr);
		fprintf(stderr, "\n");
	}
	exit(EXIT_FAILURE);
}

Object *Number_Add(Object *x, Object *y) {
	return Number_BinaryOp(x, y, ADD);
}

Object *Number_Sub(Object *x, Object *y) {
	return Number_BinaryOp(x, y, SUB);
}

Object *Number_Mul(Object *x, Object *y) {
	return Number_BinaryOp(x, y, MUL);
}

Object *Number_Div(Object *x, Object *y) {
	return Number_BinaryOp(x, y, DIV);
}

int Number_Eqv(Object *x, Object *y) {
	return TYPE(x)->tp_cmp->cmp_eqv(x, y);
}

int Number_EQ(Object *x, Object *y) {
	return NUMBER_METHODS(x)->nb_eq(x, y);
}

int Number_LT(Object *x, Object *y) {
	return NUMBER_METHODS(x)->nb_lt(x, y);
}

int Number_LE(Object *x, Object *y) {
	return Number_EQ(x, y) || Number_LT(x, y);
}

int Number_GT(Object *x, Object *y) {
	return Number_LT(y, x);
}

int Number_GE(Object *x, Object *y) {
	return Number_LE(y, x);
}
//Object *Number_Pow(Object *x, Object *y) {
//	/*
//	 * (expt 0 0) = 1
//	 * (expt _ 0) = 1
//	 * (expt 0 _) = 0
//	 * (expt _ 0.) = 1.
//	 * (expt m n)
//	 * (expt q n)
//	 *
//	 * n in Z
//	 * m = 1, 2, 3, ...
//	 * n ** m = well-defined
//	 * (q/p) ** m = q ** m / p ** m
//	 * x ** m = well-defined
//	 * z ** m = well-defined
//	 *
//	 * n ** 0 = 1
//	 * (q/p) ** 0 = 1
//	 * x ** 0 = 1
//	 * z ** 0 = 1
//	 *
//	 * n ** -m  = Z
//	 * { (q/p) ** m | q in Z , p & m in N} = q ** m / p ** m
//	 * { x ** m | x in R, m in N } = R
//	 * { z ** m | z in C, m in N } = C
//	 *
//	 * exact-fraction ** positive-exact-integer =
//	 */
//	pow(1., 0.);
//	if (IsNumber(x)) {
//		if (IsNumber(y)) {
//			if (IsZero(y)) {
//				if (IsExact(y)) {
//					return AS_OBJECT(Long_From_i64(1));
//				} else {
//					return AS_OBJECT(Float_New(1.));
//				}
//			} else {
//				if(IsZero(x)){
//					if(IsExact(x)){
//						return AS_OBJECT(Long_From_i64(0));
//					}
//				}
//
//			}
//		} else {
////			char opcode_name = "+-*/"[opcode];
//			fprintf(stderr, "wrong type argument in position 2: ");
//			TYPE(y)->tp_print(y, stderr);
//			fprintf(stderr, "\n");
//		}
//	} else {
//		char opcode_name = "+-*/"[opcode];
//		fprintf(stderr, "wrong type argument in position 1: ");
//		TYPE(x)->tp_print(x, stderr);
//		fprintf(stderr, "\n");
//	}
//	exit(EXIT_FAILURE);
//}

#undef call_unaryfunc
