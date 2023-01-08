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
#include "parser.h"

TypeObject Procedure_Type = {
		"procedure",
		.tp_basicsize=sizeof(Procedure_Type),
		.tp_itemsize=0,
		.tp_print=(print_proc) Procedure_Print,
		.tp_dealloc=(dealloc_proc) Procedure_Dealloc,
		.tp_flags=TPFLAGS_HAVE_GC,
		.tp_traverse=(traverse_proc) Procedure_Traverse,
		.tp_search=(search_proc) Procedure_Search
};

ProcedureObject *Procedure_New(Object *formals, CodeObject *code, ChainMap *lexical_scope) {
	assert(Procedure_FormalsCheckValid(formals));
	assert(Procedure_FormalsCheckUnique(formals));
	ProcedureObject *res = AS_PROCEDURE(TypeGenericAlloc(&Procedure_Type, 0));
	res->formals = NewRef(formals);
	res->code = NEW_REF(code);
	res->c_func_name = NULL;
	res->c_function = NULL;
	res->lexical_scope = NEW_REF(lexical_scope);
	gc_track(AS_OBJECT(res));
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
	gc_track(AS_OBJECT(res));
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

void Procedure_GC_Clear(ProcedureObject *self) {
	if (self->c_function == NULL) {
		CLEAR(self->formals);
		CLEAR(self->code);
		CLEAR(self->lexical_scope);
		assert(self->c_func_name == NULL);
	} else {
		CLEAR(self->c_func_name);
		assert(self->formals == NULL);
		assert(self->code == NULL);
		assert(self->lexical_scope == NULL);
	}

}

void Procedure_Dealloc(ProcedureObject *self) {
	gc_untrack(AS_OBJECT(self));
	Procedure_GC_Clear(self);
}

void Procedure_Traverse(ProcedureObject *self, visit_proc visit, void *arg) {
	if (self->c_function == NULL) {
		VISIT(self->formals, arg);
		VISIT(self->code, arg);
		VISIT(self->lexical_scope, arg);
		assert(self->c_func_name == NULL);
	} else {
		VISIT(self->c_func_name, arg);
	}
}

void Procedure_Search(ProcedureObject *self, Object *target, ArrayObject *res) {
	APPEND_PARENT(target, self, self->formals);
	APPEND_PARENT(target, self, self->code);
	APPEND_PARENT(target, self, self->c_func_name);
	APPEND_PARENT(target, self, self->lexical_scope);
	SEARCH(self->formals, target, res);
	SEARCH(self->code, target, res);
	SEARCH(self->c_func_name, target, res);
	SEARCH(self->lexical_scope, target, res);
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
