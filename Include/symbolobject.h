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

SymbolObject *Symbol_Format(const char *format, ...);

int Symbol_Eqv(SymbolObject *self, Object *other);

int Symbol_Eqv_CStr(SymbolObject *self, const char *other);

void Symbol_Repr(SymbolObject *, FILE *);

void Symbol_Dealloc(SymbolObject *);

void Symbol_Search(SymbolObject *self, Object *target, ArrayObject *res);


#endif