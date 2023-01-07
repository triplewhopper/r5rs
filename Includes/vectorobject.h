#ifndef R5RS_VECTOROBJECT_H
#define R5RS_VECTOROBJECT_H

#include "object.h"

struct vector_object {
	VarObject ob_base;
	Object *ob_item[1];
};
extern TypeObject Vector_Type;

VectorObject *Vector_New(size_t size);

size_t Vector_GetSize(VectorObject *self);

int Vector_Equal(VectorObject *self, VectorObject *other);

void Vector_Print(VectorObject *self, FILE *out);

void Vector_Dealloc(VectorObject *self);

void Vector_Traverse(VectorObject *self, visit_proc visit, void *arg);

void Vector_Search(VectorObject *self, Object *target, ArrayObject *res);

// (self: &vec, index: u32) -> &obj
Object *Vector_GetItem(VectorObject *self, size_t index);

// (self: &mut vec, index: u32, item: obj) -> void
void Vector_SetItem(VectorObject *self, size_t index, Object *item);


#endif //R5RS_VECTOROBJECT_H
