#include <stdlib.h>
#include "../Include/typeobject.h"
#include "../Include/pairobject.h"
#include "../Include/procedureobject.h"
#include "../Include/noneobject.h"

CompareMethods pair_compare = {
		.cmp_equal = (int_binaryfunc) Pair_Equal,
};
TypeObject Pair_Type = {
		"pair",
		.tp_basicsize=sizeof(PairObject),
		.tp_itemsize=0,
		.tp_print=(print_proc) Pair_Print,
		.tp_dealloc=(dealloc_proc) Pair_Dealloc,
//		.tp_eval=(evalfunc)Pair_Eval,
};
TypeObject EmptyList_Type = {
		"empty_list",
		.tp_basicsize=sizeof(PairObject),
		.tp_itemsize=0,
		.tp_print=(print_proc) EmptyList_Print,
		.tp_repr = (print_proc) EmptyList_Print,
};

PairObject empty_list = {
		{1, &EmptyList_Type}, NULL, NULL, .is_list=1
};

//steals car, cdr
PairObject *Pair_New(Object *car, Object *cdr) {
	PairObject *res = (PairObject *) TypeGenericAlloc(&Pair_Type, 0);
	assert(car);
	assert(cdr);
	res->car = car;
	res->cdr = cdr;
	if (IS_NULL(cdr) || IS_TYPE(cdr, Pair_Type)) {
		res->is_list = AS_PAIR(cdr)->is_list;
	} else res->is_list = 0;
	return res;
}

void Pair_Print(PairObject *self, FILE *out) {
	Object *car = CAR(self);
	Object *cdr = CDR(self);
	fprintf(out, "(");
	REPR(car, out);
	if (self->is_list) {
		while (!IS_NULL(cdr)) {
			fprintf(out, " ");
			REPR(CAR(cdr), out);
			cdr = CDR(cdr);
		}
	} else {
		fprintf(out, " . ");
		REPR(cdr, out);
	}

	fprintf(out, ")");
}

void Pair_Repr(PairObject *self, FILE *out) {
	Object *car = CAR(self);
	Object *cdr = CDR(self);
	fprintf(out, "(");
	REPR(car, out);
	if (self->is_list) {
		while (!IS_NULL(cdr)) {
			fprintf(out, " ");
			REPR(CAR(cdr), out);
			cdr = CDR(cdr);
		}
	} else {
		fprintf(out, " . ");
		REPR(cdr, out);
	}

	fprintf(out, ")");
}

void Pair_Dealloc(PairObject *self) {
	DECREF(self->car);
	DECREF(self->cdr);
}

// steals self
Object *Pair_GetCar(PairObject *self) {
	assert(IS_OWNED(self));
	Object *res = self->car;
	self->car = NewRef(OBJ_NONE);
	DECREF(self);
	return res;
}

// steals self
Object *Pair_GetCdr(PairObject *self) {
	assert(IS_OWNED(self));
	Object *res = self->cdr;
	self->cdr = NewRef(OBJ_NONE);
	DECREF(self);
	return res;
}

int Pair_IsList(PairObject *self) {
	return self->is_list;
}

size_t Pair_ListLength(Object *self) {
	assert(IS_LIST(self));
	size_t ans = 0;
	for (Object *cdr = self; IS_NOT_NULL(cdr); cdr = CDR(cdr)) ans++;
	return ans;
}

int Pair_Equal(PairObject *self, PairObject *other) {
	assert(0);
}

void EmptyList_Print(PairObject *self, FILE *out) {
	assert(self == &empty_list);
	fprintf(out, "()");
}

//void Pair_Eval(PairObject *self, VirtualMachineObject *vm){
//	EVAL(CAR(self), vm);
//	if(IS_TYPE(car, Symbol_Type)){
//		if(String_EqualCStyleStr(CAST(SymbolObject*, car)->name, "quote")){
//			return EVAL(CDR(self), env);
//		} else if(String_EqualCStyleStr(CAST(SymbolObject*, car)->name, "lambda")){
//			return Procedure_New(self, )
//		}
//	} else if (IS_LIST(car)){
//
//	}
//	DECREF(car);
//}