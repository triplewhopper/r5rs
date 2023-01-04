#ifndef _R5RS_DICT_OBJECT_H_
#define _R5RS_DICT_OBJECT_H_

#include "object.h"
#include "typeobject.h"

struct dict_object {
	VarObject ob_base;
	struct dict_entry_object *head;
};

DictObject *Dict_New();

void Dict_Print(DictObject *, FILE *);

void Dict_Traverse(DictObject *);
void Dict_Dealloc(DictObject *);
size_t Dict_GetSize(DictObject *);

void Dict_SetItem(DictObject *, SymbolObject *, Object *);

Object *Dict_GetItem(DictObject *, SymbolObject *);

void Dict_DelItem(DictObject *self, SymbolObject *key);

int Dict_Contains(DictObject *, SymbolObject *);

#endif // _R5RS_DICT_OBJECT_H_