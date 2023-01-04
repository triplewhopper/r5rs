#ifndef R5RS_SYMBOL_OBJECT_H
#define R5RS_SYMBOL_OBJECT_H

#include "object.h"

// extends StringObject
struct symbol_object {
	Object ob_base;
	StringObject *name;
};
extern TypeObject Symbol_Type;

SymbolObject *Symbol_FromCStr(const char *);

SymbolObject *Symbol_FromString(StringObject *);

void Symbol_Print(SymbolObject *, FILE *);

int Symbol_Eqv(SymbolObject *self, Object *other);

int Symbol_Eqv_CStr(SymbolObject *self, const char *other);

void Symbol_Repr(SymbolObject *, FILE *);

void Symbol_Dealloc(SymbolObject *);

struct g_symbols {
	Object *cond;
	Object *cond_case;
	Object *define;
	Object *cond_else;
	Object *if_expr;
	Object *lambda;
	Object *let;
	Object *let_star;
	Object *set;
	Object *quote;
} global_symbols;

void GlobalSymbolsInit();

void GlobalSymbolsFinalize();

#endif