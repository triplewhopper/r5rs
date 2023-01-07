#ifndef R5RS_CHAROBJECT_H
#define R5RS_CHAROBJECT_H

#include "object.h"

struct char_object {
	Object ob_base;
	int ob_val;
};

CharObject *Char_New(int);

CharObject *Char_FromLiteral(StringObject *);

int Char_Ord(CharObject *);

void Char_Print(CharObject *, FILE *);

int Char_Eqv(CharObject *, Object *);

#endif //R5RS_CHAROBJECT_H
