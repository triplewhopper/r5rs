#include <stdlib.h>
#include "object.h"
#include "symbolobject.h"
#include "interpreter.h"
#include "builtins.h"
#include "chainmapobject.h"

static R5RS *self = NULL;


R5RS *R5RS_New() {
	if (self == NULL) {
		self = calloc(1, sizeof(R5RS));
		self->keywords = &global_symbols;
		self->builtins = load_builtins();
		self->globals = ChainMap_NewChild(self->builtins, NULL);
	}
	return self;
}

void R5RS_SetGlobalItem(R5RS *self, SymbolObject *key, Object *value) {
	ChainMap_SetItem(self->globals, key, value);
}

Object *R5RS_GetGlobalItem(R5RS *self, SymbolObject *key) {
	return ChainMap_GetItem(self->globals, key);
}

void R5RS_Dealloc(R5RS *r) {
	assert(r);
	assert(r == self);
	self->keywords = NULL;
	CLEAR(self->globals);
	CLEAR(self->builtins);
	free(self);
	self = NULL;
}
void global_symbols_init() {
	global_symbols.cond_case = AS_OBJECT(Symbol_FromCStr("case"));
	global_symbols.cond = AS_OBJECT(Symbol_FromCStr("cond"));
	global_symbols.define = AS_OBJECT(Symbol_FromCStr("define"));
	global_symbols.cond_else = AS_OBJECT(Symbol_FromCStr("else"));
	global_symbols.cond_arrow = AS_OBJECT(Symbol_FromCStr("=>"));
	global_symbols.if_expr = AS_OBJECT(Symbol_FromCStr("if"));
	global_symbols.lambda = AS_OBJECT(Symbol_FromCStr("lambda"));
	global_symbols.let = AS_OBJECT(Symbol_FromCStr("let"));
	global_symbols.set = AS_OBJECT(Symbol_FromCStr("set!"));
	global_symbols.quote = AS_OBJECT(Symbol_FromCStr("quote"));
}

void global_symbols_finalize() {
	DECREF(global_symbols.cond_case);
	DECREF(global_symbols.cond);
	DECREF(global_symbols.define);
	DECREF(global_symbols.cond_else);
	DECREF(global_symbols.cond_arrow);
	DECREF(global_symbols.if_expr);
	DECREF(global_symbols.lambda);
	DECREF(global_symbols.let);
	DECREF(global_symbols.set);
	DECREF(global_symbols.quote);
}
