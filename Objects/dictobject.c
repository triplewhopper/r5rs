#include "../Include/typeobject.h"
#include "../Include/dictobject.h"
#include "../Include/symbolobject.h"
#include "../Include/arrayobject.h"

struct dict_entry_object {
//	Object ob_base;
	SymbolObject *key;
	Object *value;
};

struct dict_object {
	VarObject ob_base;
	ArrayObject *items;
};


//static DictNode *DictNode_New(SymbolObject *, Object *);

//static void DictNode_Dealloc(DictNode *);

//static void DictNode_Search(DictNode *, Object *, ArrayObject *);

//static void DictNode_Traverse(DictNode *, visit_proc visit, void *arg);

//TypeObject DictNode_Type = {
//		.tp_name="dictNode",
//		.tp_basicsize =sizeof(DictNode),
//		.tp_itemsize=0,
//		.tp_print=(print_proc) NULL,
//		.tp_dealloc=(dealloc_proc) DictNode_Dealloc,
//		.tp_search=(search_proc) DictNode_Search,
////		.tp_flags=TPFLAGS_HAVE_GC,
////		.tp_traverse=(traverse_proc) DictNode_Traverse,
//};

//DictNode *DictNode_New(SymbolObject *key, Object *value) {
//	assert(key != NULL);
//	assert(value != NULL);
//	DictNode *res = (DictNode *) TypeGenericAlloc(&DictNode_Type, 0);
//	res->key = NEW_REF(key);
//	res->value = NEW_REF(value);
////	gc_track(AS_OBJECT(res));
//	return res;
//}

//void DictNode_Dealloc(DictNode *self) {
////	gc_untrack(AS_OBJECT(self));
//	CLEAR(self->key);
//	CLEAR(self->value);
//}
//
//void DictNode_Search(DictNode *self, Object *target, ArrayObject *res) {
//	APPEND_PARENT(target, self, self->key);
//	APPEND_PARENT(target, self, self->value);
//	SEARCH(self->key, target, res);
//	SEARCH(self->value, target, res);
//}

//void DictNode_Traverse(DictNode *self, visit_proc visit, void *arg) {
//	VISIT(self->key, arg);
//	VISIT(self->value, arg);
//}

TypeObject Dict_Type = {
		.tp_name="dict",
		.tp_basicsize =sizeof(DictObject),
		.tp_itemsize=sizeof(DictNode),
		.tp_print=(print_proc) Dict_Print,
		.tp_dealloc=(dealloc_proc) Dict_Dealloc,
		.tp_flags = TPFLAGS_HAVE_GC,
		.tp_traverse=(traverse_proc) Dict_Traverse,
		.tp_search=(search_proc) Dict_Search,
};

#define DictNodeAt(self, index) Array_At(DictNode, (self)->items, (index))

DictObject *Dict_New() {
	DictObject *res = (DictObject *) TypeGenericAlloc(&Dict_Type, 0);
	SIZE(res) = 0;
	res->items = NULL;
	gc_track(AS_OBJECT(res));
	return res;
}

void Dict_Print(DictObject *self, FILE *out) {
	size_t sz = SIZE(self);
	if (sz > 0) {
		fputc('{', out);
		for (size_t i = 0; i < SIZE(self); ++i) {
			DictNode *const node = &DictNodeAt(self, i);
			PRINT(node->key, out);
			fputs(": ", out);
			PRINT(node->value, out);
			if (i + 1 != SIZE(self)) {
				fputs(", ", out);
			}
		}
		fputc('}', out);
	} else {
		fprintf(out, "{}");
	}
}

void Dict_Traverse(DictObject *self, visit_proc visit, void *arg) {
	for (size_t i = 0; i < SIZE(self); ++i) {
		DictNode *const node = &DictNodeAt(self, i);
		VISIT(node->key, arg);
		VISIT(node->value, arg);
	}
	VISIT(self->items, NULL);
}

void Dict_GC_Clear(DictObject *self) {
	for (size_t i = 0; i < SIZE(self); ++i) {
		DictNode *node = &DictNodeAt(self, i);
		CLEAR(node->key);
		CLEAR(node->value);
	}
	CLEAR(self->items);
}

void Dict_Search(DictObject *self, Object *target, ArrayObject *res) {
	for (size_t i = 0; i < SIZE(self); ++i) {
		DictNode *node = &DictNodeAt(self, i);
		APPEND_PARENT(target, self, node->key);
		APPEND_PARENT(target, self, node->value);
	}
	APPEND_PARENT(target, self, self->items);
	for (size_t i = 0; i < SIZE(self); ++i) {
		DictNode *node = &DictNodeAt(self, i);
		SEARCH(node->key, target, res);
		SEARCH(node->value, target, res);
	}
	SEARCH(self->items, target, res);
}

void Dict_Dealloc(DictObject *self) {
	gc_untrack(AS_OBJECT(self));
	Dict_GC_Clear(self);
}

static size_t find(DictObject *self, SymbolObject *key) {
	size_t i = 0;
	for (; i < SIZE(self); ++i) {
		DictNode *const node = &DictNodeAt(self, i);
		if (EQV(key, node->key)) {
			return i;
		}
	}
	return i;

}

void Dict_SetItem(DictObject *self, SymbolObject *key, Object *value) {
	assert(value);
	size_t i = find(self, key);
	if (i < SIZE(self)) {
		Object *tmp = NULL;
		MOVE_SET(tmp, (&DictNodeAt(self, i))->value, NewRef(value));
		return;
	} else {
		SIZE(self)++;
		if (self->items != NULL) {
			Array_Append(self->items, &(DictNode) {NEW_REF(key), NEW_REF(value)});
		} else {
			self->items = Array_New(0, sizeof(DictNode));
			Array_Append(self->items, &(DictNode) {NEW_REF(key), NEW_REF(value)});
		}
	}
}

Object *Dict_GetItem(DictObject *self, SymbolObject *key) {
	size_t i = find(self, key);
	if (i < SIZE(self)) {
		return NewRef(DictNodeAt(self, i).value);
	}
	return NULL;
}

void Dict_DelItem(DictObject *self, SymbolObject *key) {
	size_t i = find(self, key);
	if (i < SIZE(self)) {
		DictNode *node = &DictNodeAt(self, i);
		DECREF(node->key);
		DECREF(node->value);
		Array_Remove(self->items, i);
	}
}

int Dict_Contains(DictObject *self, SymbolObject *key) {
	return find(self, key) < SIZE(self);
}

#undef DictNodeAt

