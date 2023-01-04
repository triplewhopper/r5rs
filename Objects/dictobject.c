#include "../Include/typeobject.h"
#include "../Include/dictobject.h"
#include "../Include/symbolobject.h"

struct dict_entry_object {
	Object ob_base;
	SymbolObject *key;
	Object *value;
	struct dict_entry_object *next;
};


static DictNode *DictNode_New(SymbolObject *, Object *, DictNode *);

static void DictNode_Dealloc(DictNode *);

TypeObject DictNode_Type = {
		.tp_name="dictNode",
		.tp_basicsize =sizeof(DictNode),
		.tp_itemsize=0,
		.tp_print=(print_proc) NULL,
		.tp_dealloc=(dealloc_proc) DictNode_Dealloc,
};

DictNode *DictNode_New(SymbolObject *key, Object *value, DictNode *next) {
	assert(key != NULL);
	assert(value != NULL);
	DictNode *res = (DictNode *) TypeGenericAlloc(&DictNode_Type, 0);

	res->key = NEW_REF(key);
	res->value = NEW_REF(value);
	if (next) {
		res->next = NEW_REF(next);
	} else {
		res->next = NULL;
	}

	return res;
}


void DictNode_Dealloc(DictNode *self) {
	DECREF(self->key);
	DECREF(self->value);
	XDECREF(self->next);
}


TypeObject Dict_Type = {
		.tp_name="dict",
		.tp_basicsize =sizeof(DictObject),
		.tp_itemsize=sizeof(DictNode),
		.tp_print=(print_proc) Dict_Print,
		.tp_dealloc=(dealloc_proc) Dict_Dealloc,
};

DictObject *Dict_New() {
	DictObject *res = (DictObject *) TypeGenericAlloc(&Dict_Type, 0);
	res->head = NULL;
	return res;
}

void Dict_Print(DictObject *self, FILE *out) {
	size_t sz = SIZE(self);
	if (sz > 0) {
		fputc('{', out);
		for (DictNode *it = self->head; it; it = it->next) {
			PRINT(it->key, out);
			fputs(": ", out);
			PRINT(it->value, out);
			if (it->next != NULL) {
				fputs(", ", out);
			}
		}
		fputc('}', out);
	} else {
		fprintf(out, "{}");
	}
}

void Dict_Dealloc(DictObject *self) {
	XDECREF(self->head);
}

size_t Dict_GetSize(DictObject *self) {
	return self->ob_base.ob_size;
}

static DictNode **find(DictObject *self, SymbolObject *key) {
	for (DictNode **it = &self->head; *it != NULL; it = &(*it)->next) {
		if (EQV(key, (*it)->key)) {
			return it;
		}
	}
	return NULL;

}

void Dict_SetItem(DictObject *self, SymbolObject *key, Object *value) {
	assert(value);
	DictNode **entry = find(self, key);
	if (entry != NULL) {
		Object *tmp = NULL;
		MOVE_SET(tmp, (*entry)->value, NewRef(value));
		return;
	} else {
		self->ob_base.ob_size++;
		if (self->head != NULL) {
			DictNode *tmp = NULL;
			MOVE_SET(tmp, self->head, DictNode_New(key, value, self->head));
		} else {
			self->head = DictNode_New(key, value, NULL);
		}
	}
}

Object *Dict_GetItem(DictObject *self, SymbolObject *key) {
	DictNode **entry = find(self, key);
	if (entry != NULL) {
		return NewRef((*entry)->value);
	} else {
		return NULL;
	}
}

void Dict_DelItem(DictObject *self, SymbolObject *key) {
	DictNode **entry = find(self, key);
	if (entry != NULL) {
		self->ob_base.ob_size--;
		DictNode *tmp = *entry;
		*entry = (*entry)->next;
		tmp->next = NULL;
		DECREF(tmp);
	} else return;
}

int Dict_Contains(DictObject *self, SymbolObject *key) {
	return find(self, key) != NULL;
}

