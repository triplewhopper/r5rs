#include "typeobject.h"
#include "stringobject.h"
#include "symbolobject.h"
#include "dictobject.h"
#include "arrayobject.h"
#include <assert.h>
#include <stdarg.h>

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
//	fprintf(out, "'");
	PRINT(self->name, out);
//	fprintf(out, "`");
}

SymbolObject *Symbol_Format(const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	StringObject *str = String_VFormat(format, ap);
	SymbolObject *res = Symbol_FromString(str);
	DECREF(str);
	va_end(ap);
	return res;
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
//	fprintf(out, "'");
	PRINT(self->name, out);
//	fprintf(out, "`");
}

void Symbol_Dealloc(SymbolObject *self) {
	DECREF(self->name);
}

void Symbol_Search(SymbolObject *self, Object *target, ArrayObject *res) {
	APPEND_PARENT(target, self, self->name);
	SEARCH(self->name, target, res);
}

