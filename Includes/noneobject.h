#ifndef R5RS_NONEOBJECT_H
#define R5RS_NONEOBJECT_H

#include "object.h"

struct none_object {
	Object ob_base;
};
extern TypeObject None_Type;
extern NoneObject None;
#define OBJ_NONE AS_OBJECT(&None)
#define IS_NONE(obj) (AS_OBJECT(obj)==OBJ_NONE)
#define IS_NOT_NONE(obj) (AS_OBJECT(obj)!=OBJ_NONE)
#define RETURN_NONE do{INCREF(OBJ_NONE);return OBJ_NONE;}while(0)
void None_Print(Object *, FILE *);

void None_Dealloc(NoneObject *self);


#endif //R5RS_NONEOBJECT_H
