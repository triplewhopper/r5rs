#ifndef R5RS_TYPE_OBJECT_H
#define R5RS_TYPE_OBJECT_H

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "typedefs.h"

//#define __DEBUG__
#define TPFLAGS_HAVE_GC 1

struct number_methods {
	binaryfunc nb_add;
	binaryfunc nb_sub;
	binaryfunc nb_mul;
	binaryfunc nb_div;
	binaryfunc nb_pow;
	unaryfunc nb_neg;

	binaryfunc nb_lshift;
	binaryfunc nb_rshift;


	unaryfunc nb_long;
	unaryfunc nb_float;
	unaryfunc nb_fraction;
	unaryfunc nb_complex;

	int_unaryfunc nb_is_zero;
	int_unaryfunc nb_is_exact;
	int_unaryfunc nb_is_inexact;
	unaryfunc nb_to_exact;
	unaryfunc nb_to_inexact;


	int_unaryfunc nb_is_integer;
	int_unaryfunc nb_is_rational;
	int_unaryfunc nb_is_real;
	int_unaryfunc nb_is_complex;

	int_binaryfunc nb_eq;
	int_binaryfunc nb_ne;
	int_binaryfunc nb_gt;
	int_binaryfunc nb_ge;
	int_binaryfunc nb_lt;
	int_binaryfunc nb_le;
};
struct cmp_methods {
	int_binaryfunc cmp_eq;
	int_binaryfunc cmp_eqv;
	int_binaryfunc cmp_equal;
};

struct type_object {
	const char *const tp_name;
	const size_t tp_basicsize;
	const size_t tp_itemsize;

	Object *(*tp_alloc)(TypeObject *, size_t);

	print_proc tp_print;
	print_proc tp_repr;
	dealloc_proc tp_dealloc;

	NumberMethods *tp_as_number;
	CompareMethods *tp_cmp;
	size_t tp_flags;
	traverse_proc tp_traverse;
//	int_unaryfunc tp_clear;
	search_proc tp_search;

	struct type_object *tp_base;
};



void Type_InitTypeObjects();

extern TypeObject BaseObject_Type;

extern TypeObject Boolean_Type;
extern TypeObject Long_Type;
extern TypeObject Float_Type;
extern TypeObject Fraction_Type;
extern TypeObject Complex_Type;
extern TypeObject Pair_Type;
extern TypeObject None_Type;
extern TypeObject Char_Type;
extern TypeObject String_Type;
extern TypeObject Symbol_Type;
extern TypeObject Vector_Type;
extern TypeObject Procedure_Type;

extern TypeObject Token_Type;
extern TypeObject Dict_Type;
extern TypeObject Array_Type;
extern TypeObject ChainMap_Type;

extern TypeObject Lexer_Type;
extern TypeObject Code_Type;
extern TypeObject Frame_Type;
extern TypeObject VirtualMachine_Type;



#define PRINT(obj, out) TYPE(obj)->tp_print(AS_OBJECT(obj), out)

#ifdef __DEBUG__
#define DEBUG(obj, out) PRINT(obj, out)
#else
#define DEBUG(obj, out)
#endif

#define REPR(obj, out) TYPE(obj)->tp_repr(AS_OBJECT(obj), out)
#define TRAVERSE(obj, visit, arg) TYPE(obj)->tp_traverse(AS_OBJECT(obj), visit, arg)
#define VISIT(obj, arg) visit(AS_OBJECT(obj), arg)
#define SEARCH(obj, target, res) \
    do{ \
        if((obj) != NULL && !AS_OBJECT(obj)->searched){ \
            AS_OBJECT(obj)->searched = 1; \
            if(TYPE(obj)->tp_search) TYPE(obj)->tp_search(AS_OBJECT(obj), target, res); \
        } \
    }while(0)

#define APPEND_PARENT(target, parent, son) do{if(AS_OBJECT(son) == (target)) Array_Append(res, &(parent));}while(0)
#endif //R5RS_TYPE_OBJECT_H