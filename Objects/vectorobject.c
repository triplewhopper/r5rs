#include <stdlib.h>
#include "../Include/typeobject.h"
#include "../Include/vectorobject.h"
#include "../Include/noneobject.h"
#include "../Include/arrayobject.h"

TypeObject Vector_Type = {
		.tp_name = "Vector",
		.tp_basicsize=sizeof(VectorObject),
		.tp_itemsize=sizeof(Object *),
		.tp_print=(print_proc) Vector_Print,
		.tp_dealloc=(dealloc_proc) Vector_Dealloc,
		.tp_flags=TPFLAGS_HAVE_GC,
		.tp_traverse=(traverse_proc) Vector_Traverse,
		.tp_search=(search_proc) Vector_Search,
};

VectorObject *Vector_New(size_t size) {
	VectorObject *res = (VectorObject *) TypeGenericAlloc(&Vector_Type, size);
	assert(SIZE(res) == size);
	for (size_t i = 0; i < size; ++i) {
		res->ob_item[i] = NewRef(OBJ_NONE);
	}
	gc_track(AS_OBJECT(res));
	return res;
}

size_t Vector_GetSize(VectorObject *self) {
	return self->ob_base.ob_size;
}

int Vector_Equal(VectorObject *self, VectorObject *other) {
	if (Vector_GetSize(self) != Vector_GetSize(other)) {
		return 0;
	}
	assert(0);
}

// this function returns a borrowed ref to item.
Object *Vector_GetItem(VectorObject *self, size_t index) {
	if (0 <= index && index < Vector_GetSize(self)) {
		return self->ob_item[index];
	}
	fprintf(stderr, "in function %s: IndexError: index out of bound\n", __FUNCTION__);
	exit(EXIT_FAILURE);
}

// this function steals ref to item, and the original ref is discarded.
void Vector_SetItem(VectorObject *self, size_t index, Object *item) {
	if (0 <= index && index < Vector_GetSize(self)) {
		Object *tmp = NULL;
		MOVE_SET(tmp, self->ob_item[index], item);
		return;
	}
	fprintf(stderr, "in function %s: IndexError: index out of bound\n", __FUNCTION__);
	exit(EXIT_FAILURE);
}

void Vector_Print(VectorObject *self, FILE *out) {
	if (Vector_GetSize(self)) {
		fprintf(out, "#(");
		TYPE(self->ob_item[0])->tp_print(self->ob_item[0], out);
		for (size_t i = 1; i < self->ob_base.ob_size; ++i) {
			fputc(' ', out);
			PRINT(self->ob_item[i], out);
		}
		fprintf(out, ")");
	} else {
		fprintf(out, "#()");
		return;
	}

}

void Vector_Dealloc(VectorObject *self) {
	gc_untrack(AS_OBJECT(self));
	for (size_t i = 0; i < SIZE(self); ++i) {
		CLEAR(self->ob_item[i]);
	}
}

void Vector_Traverse(VectorObject *self, visit_proc visit, void *arg){
	for (size_t i = 0; i < SIZE(self); ++i) {
		VISIT(self->ob_item[i], arg);
	}
}

void Vector_Search(VectorObject *self, Object *target, ArrayObject *res){
	for (size_t i = 0; i < SIZE(self); ++i) {
		APPEND_PARENT(target, self, self->ob_item[i]);
	}
	for (size_t i = 0; i < SIZE(self); ++i) {
		SEARCH(self->ob_item[i], target, res);
	}

}

