#include "../Include/typeobject.h"
#include "../Include/stringobject.h"
#include "../Include/symbolobject.h"
#include "../Include/dictobject.h"
#include "../Include/arrayobject.h"
#include <assert.h>

CompareMethods symbol_compare = {
		.cmp_eq = (int_binaryfunc) Symbol_Eqv,
		.cmp_eqv = (int_binaryfunc) Symbol_Eqv,
};
TypeObject Symbol_Type = {
		.tp_name="symbol",
		.tp_basicsize=sizeof(SymbolObject),
		.tp_itemsize=1,
		.tp_print=(print_proc) Symbol_Print,
		.tp_repr=(print_proc) Symbol_Repr,
		.tp_dealloc = (dealloc_proc) Symbol_Dealloc,
		.tp_cmp = &symbol_compare,
		.tp_search=(search_proc) Symbol_Search
};

SymbolObject *Symbol_FromCStr(const char *s) {
	SymbolObject *res = AS_SYMBOL(TypeGenericAlloc(&Symbol_Type, 0));
	res->name = String_FromCStr(s);
	return res;
}

SymbolObject *Symbol_FromString(StringObject *s) {
	SymbolObject *res = AS_SYMBOL(TypeGenericAlloc(&Symbol_Type, 0));
	res->name = String_FromCStrN(s->ob_sval, String_GetSize(s));
	return res;
}

void Symbol_Print(SymbolObject *self, FILE *out) {
	fprintf(out, "`");
	PRINT(self->name, out);
	fprintf(out, "`");
}

int Symbol_Eqv(SymbolObject *self, Object *other) {
	if (IS_TYPE(other, Symbol_Type)) {
		return EQV(self->name, AS_SYMBOL(other)->name);
	}
	return 0;
}

int Symbol_Eqv_CStr(SymbolObject *self, const char *other) {
	return String_EqualCStyleStr(self->name, other);
}

void Symbol_Repr(SymbolObject *self, FILE *out) {
	fprintf(out, "`");
	PRINT(self->name, out);
	fprintf(out, "`");
}

void Symbol_Dealloc(SymbolObject *self) {
	DECREF(self->name);
}

void Symbol_Search(SymbolObject *self, Object *target, ArrayObject *res){
	APPEND_PARENT(target, self, self->name);
	SEARCH(self->name, target, res);
}

void GlobalSymbolsInit() {
	global_symbols.cond_case = AS_OBJECT(Symbol_FromCStr("case"));
	global_symbols.cond = AS_OBJECT(Symbol_FromCStr("cond"));
	global_symbols.define = AS_OBJECT(Symbol_FromCStr("define"));
	global_symbols.cond_else = AS_OBJECT(Symbol_FromCStr("else"));
	global_symbols.if_expr = AS_OBJECT(Symbol_FromCStr("if"));
	global_symbols.lambda = AS_OBJECT(Symbol_FromCStr("lambda"));
	global_symbols.let = AS_OBJECT(Symbol_FromCStr("let"));
	global_symbols.set = AS_OBJECT(Symbol_FromCStr("set!"));
	global_symbols.quote = AS_OBJECT(Symbol_FromCStr("quote"));
}

void GlobalSymbolsFinalize() {
	DECREF(global_symbols.cond_case);
	DECREF(global_symbols.cond);
	DECREF(global_symbols.define);
	DECREF(global_symbols.cond_else);
	DECREF(global_symbols.if_expr);
	DECREF(global_symbols.lambda);
	DECREF(global_symbols.let);
	DECREF(global_symbols.set);
	DECREF(global_symbols.quote);
}
