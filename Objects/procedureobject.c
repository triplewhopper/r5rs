#include <stdlib.h>
#include "../Include/typeobject.h"
#include "../Include/pairobject.h"
#include "../Include/chainmapobject.h"
#include "../Include/symbolobject.h"
#include "../Include/stringobject.h"
#include "../Include/procedureobject.h"
#include "../Include/numberobject.h"
#include "../Include/noneobject.h"
#include "../Include/codeobject.h"
#include "../Parser/parser.h"

TypeObject Procedure_Type = {
		"procedure",
		.tp_basicsize=sizeof(Procedure_Type),
		.tp_itemsize=0,
		.tp_print=(print_proc) Procedure_Print,
		.tp_dealloc=(dealloc_proc) Procedure_Dealloc,
};

ProcedureObject *Procedure_New(Object *formals, CodeObject *code, ChainMap *lexical_scope) {
	assert(IS_LIST(formals) || IS_TYPE(formals, Pair_Type) || IS_TYPE(formals, Symbol_Type));
	ProcedureObject *res = AS_PROCEDURE(TypeGenericAlloc(&Procedure_Type, 0));
	res->formals = NewRef(formals);
	res->code = NEW_REF(code);
	res->c_func_name = NULL;
	res->c_function = NULL;
	res->lexical_scope = NEW_REF(lexical_scope);
	return res;
}

ProcedureObject *Procedure_FromCFunction(StringObject *name, CFunction func) {
	assert(func != NULL);
	ProcedureObject *res = AS_PROCEDURE(TypeGenericAlloc(&Procedure_Type, 0));
	res->formals = NULL;
	res->code = NULL;
	res->c_func_name = NEW_REF(name);
	res->c_function = func;
	res->lexical_scope = NULL;
	return res;

}

int Procedure_FormalsCheckValid(Object *formals) {
	if (IS_NULL(formals) || IS_TYPE(formals, Symbol_Type)) return 1;
	if (IS_TYPE(formals, Pair_Type)) {
		Object *o = formals;
		for (; IS_TYPE(o, Pair_Type); o = CDR(o)) {
			if (!IS_TYPE(CAR(o), Symbol_Type)) return 0;
		}
		return IS_NULL(o) || IS_TYPE(o, Symbol_Type);
	}
	return 0;
}

static int is_unique(SymbolObject *symbols[], size_t n) {
	for (size_t i = 0; i < n; ++i) {
		for (size_t j = 0; j < i; ++j) {
			if (EQV(symbols[i], symbols[j])) {
				return 0;
			}
		}
	}
	return 1;
}

static size_t count_formals(Object *formals) {
	size_t ans = 0;
	for (; IS_TYPE(formals, Pair_Type); formals = CDR(formals)) {
		ans++;
	}
	if (IS_TYPE(formals, Symbol_Type)) ans++;
	return ans;
}

// must call Procedure_FormalsCheckValid() first.
int Procedure_FormalsCheckUnique(Object *formals) {
	if (IS_NULL(formals) || IS_TYPE(formals, Symbol_Type)) return 1;
	if (IS_TYPE(formals, Pair_Type)) {
		size_t argc = count_formals(formals), i;
		SymbolObject **tmp = calloc(argc, sizeof(SymbolObject *));
		assert(tmp);
		for (i = 0; IS_TYPE(formals, Pair_Type); formals = CDR(formals)) {
			assert(IS_TYPE(CAR(formals), Symbol_Type));
			tmp[i++] = AS_SYMBOL(CAR(formals));
		}
		assert(IS_NULL(formals) || IS_TYPE(formals, Symbol_Type));
		if (IS_TYPE(formals, Symbol_Type)) tmp[i++] = AS_SYMBOL(formals);
		assert(i == argc);
		int res = is_unique(tmp, argc);
		free(tmp);
		return res;
	}
	return 0;
}

void Procedure_Dealloc(ProcedureObject *self) {
	if (self->c_function == NULL) {
		DECREF(self->formals);
		DECREF(self->code);
		DECREF(self->lexical_scope);
		assert(self->c_func_name == NULL);
	} else {
		DECREF(self->c_func_name);
		assert(self->formals == NULL);
		assert(self->code == NULL);
		assert(self->lexical_scope == NULL);
	}
}

void Procedure_Print(ProcedureObject *self, FILE *f) {
	if (self->c_function) {
		fprintf(f, "<C function `");
		PRINT(self->c_func_name, f);
		fprintf(f, "`@ %p>", &self->c_function);
		return;
	}
	if (self->code->co_name) {
		fprintf(f, "<");
		PRINT(self->code->co_name, f);
		fprintf(f, " ");
	} else {
		fprintf(f, "<lambda ");
	};
	PRINT(self->formals, f);
	fprintf(f, ", code #%zu @ %p>", self->code->co_id, &self);
}

//
//Object *Form_Define(ProcedureObject *self, Object *form) {
//
//	fprintf(stderr, "in function %s: bad form: ", __FUNCTION__);
//	PRINT(form, stderr);
//	fputc('\n', stderr);
//	exit(EXIT_FAILURE);
//
//	RETURN_NONE;
//}
//
//Object *Form_Set(ProcedureObject *self, Object *form) {
//	assert(IS_LIST(form));
//	Object *var = NULL, *def_formals = NULL, *expr = NULL;
//	if (ParseArgs(form, "(set! ? ?)", &var, &expr) == 2) {
//		assert(IS_TYPE(var, Symbol_Type));
//		INCREF(expr);
//		Object *e = EVAL(expr, self->lexical_scope);
//		ChainMap_SetItem(self->lexical_scope, CAST(SymbolObject *, var), e);
//		DECREF(e);
//		DECREF(expr);
//	} else {
//		fprintf(stderr, "in function %s: bad form: ", __FUNCTION__);
//		PRINT(form, stderr);
//		fputc('\n', stderr);
//		exit(EXIT_FAILURE);
//	}
//	RETURN_NONE;
//}
//
//Object *Form_If(ProcedureObject *self, Object *form) {
//	assert(IS_LIST(form));
//	Object *test = NULL, *consequent = NULL, *alternate = NULL;
//	if (ParseArgs(form, "(if ? ? ?)", &test, &consequent, &alternate) == 3) {
//		INCREF(test);
//		INCREF(consequent);
//		INCREF(alternate);
//		Object *e = EVAL(test, self->lexical_scope);
//		DECREF(test);
//		if (!IS_FALSE(e)) {
//			DECREF(e);
//			DECREF(alternate);
//			return consequent;
//		} else {
//			DECREF(e);
//			DECREF(consequent);
//			return alternate;
//		}
//	} else if (ParseArgs(form, "(if ? ?)", &test, &consequent) == 2) {
//		INCREF(test);
//		INCREF(consequent);
//		Object *e = EVAL(test, self->lexical_scope);
//		DECREF(test);
//		if (!IS_FALSE(e)) {
//			DECREF(e);
//			return consequent;
//		} else {
//			DECREF(e);
//			DECREF(consequent);
//			RETURN_NONE;
//		}
//	} else {
//		fprintf(stderr, "in function %s: bad form:", __FUNCTION__);
//		PRINT(form, stderr);
//		fputc('\n', stderr);
//		exit(EXIT_FAILURE);
//	}
//}
//
//Object *Builtin_NumberAdd(ProcedureObject *self, Object *args) {
//	assert(IS_LIST(args));
//	if (IS_NOT_NULL(args)) {
//		Object *ans = CAR(args);
//		Object *tmp;
//		INCREF(ans);
//		for (Object *e = CDR(args); !IS_NULL(e); e = CDR(e)) {
//			MOVE_SET(tmp, ans, Number_Add(ans, CAR(e)));
//		}
//		return ans;
//	} else return AS_OBJECT(Long_From_i64(0));
//}
//
//Object *Builtin_NumberSub(ProcedureObject *self, Object *args) {
//	assert(IS_LIST(args));
//	if (IS_NOT_NULL(args)) {
//		Object *ans = CAR(args);
//		Object *tmp;
//		INCREF(ans);
//		for (Object *e = CDR(args); !IS_NULL(e); e = CDR(e)) {
//			MOVE_SET(tmp, ans, Number_Add(ans, CAR(e)));
//		}
//		return ans;
//	} else return AS_OBJECT(Long_From_i64(0));
//}