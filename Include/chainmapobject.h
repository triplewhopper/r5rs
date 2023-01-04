#ifndef R5RS_CHAINMAPOBJECT_H
#define R5RS_CHAINMAPOBJECT_H

#include "object.h"
#include "typeobject.h"
#include "dictobject.h"

struct chain_map_object {
	VarObject ob_base;
	struct dict_object *maps[1];
};
extern TypeObject ChainMap_Type;

void ChainMap_Print(ChainMap *self, FILE *out);

void ChainMap_Dealloc(ChainMap *self);

size_t ChainMap_GetSize(ChainMap *self);

ChainMap *ChainMap_New(DictObject *args[]);

ChainMap *ChainMap_NewChild(ChainMap *self, DictObject *m);

ChainMap *ChainMap_NewEmpty();

ChainMap *ChainMap_Parents(ChainMap *self);

void ChainMap_SetItem(ChainMap *self, SymbolObject *key, Object *value);

void ChainMap_DeepSetItem(ChainMap *self, SymbolObject *key, Object *value);

Object *ChainMap_GetItem(ChainMap *self, SymbolObject *key);

int ChainMap_Contains(ChainMap *self, SymbolObject *key);

void ChainMap_DelItem(ChainMap *self, SymbolObject *key);


#endif //R5RS_CHAINMAPOBJECT_H
