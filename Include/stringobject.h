#ifndef R5RS_STRING_OBJECT_H
#define R5RS_STRING_OBJECT_H

#include "object.h"

struct string_object {
	VarObject ob_base;
	shash_t ob_shash;
	char ob_sval[1];
	/* Invariants:
   *     ob_sval contains space for 'ob_size+1' elements.
   *     ob_sval[ob_size] == 0.
   *     ob_shash is the hash of the string or -1 if not computed yet.
   */

};

extern TypeObject String_Type;

StringObject *String_FromCStrN(const char *, size_t);

StringObject *String_FromCStr(const char *);

StringObject *String_FromLiteral(StringObject *);

StringObject *String_Format(const char *, ...);

size_t String_GetSize(StringObject *self);

int String_Eqv(StringObject *lhs, Object *rhs);

int String_EqualCStyleStr(StringObject *lhs, const char *rhs);

int String_Compare(StringObject *, StringObject *);

shash_t String_Hash(StringObject *);

void String_Print(StringObject *, FILE *);

void String_Repr(StringObject *, FILE *);

//void String_Dealloc(StringObject *);

StringObject *String_Add(StringObject *, StringObject *);

char String_CharAt(StringObject *, size_t);

int String_StartswithCStr(StringObject *, const char *);

int String_EndswithCtr(StringObject *, const char *);

int String_Startswith(StringObject *, StringObject *);

int String_Endswith(StringObject *, StringObject *);

#endif //R5RS_STRING_OBJECT_H