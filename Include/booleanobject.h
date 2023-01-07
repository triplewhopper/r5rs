#ifndef R5RS_BOOLEAN_OBJECT_H
#define R5RS_BOOLEAN_OBJECT_H

#include "object.h"

struct boolean_object {
	Object ob_base;
};

extern BooleanObject Boolean_True;

extern BooleanObject Boolean_False;

#define OBJ_TRUE (AS_OBJECT(&Boolean_True))
#define OBJ_FALSE (AS_OBJECT(&Boolean_False))
#define RETURN_TRUE do { INCREF(OBJ_TRUE); return OBJ_TRUE; }while(0)
#define RETURN_FALSE do { INCREF(OBJ_FALSE); return OBJ_FALSE; }while(0)

#define IS_TRUE(op) (AS_OBJECT(op) == OBJ_TRUE)
#define IS_FALSE(op) (AS_OBJECT(op) == OBJ_FALSE)

void Boolean_Print(BooleanObject *, FILE *);

void Boolean_Dealloc(BooleanObject *);

#endif //R5RS_BOOLEAN_OBJECT_H