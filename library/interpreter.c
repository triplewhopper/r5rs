#include <stdlib.h>
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