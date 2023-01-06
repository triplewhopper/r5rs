#ifndef R5RS_ARRAYOBJECT_H
#define R5RS_ARRAYOBJECT_H

#include <stdio.h>
#include <stdlib.h>
#include "object.h"
#include "typeobject.h"


#define Array_At(type, array, index) (*CAST(type*, Array_GetItem(array, index)))

extern TypeObject Array_Type;

//typedef struct array_ref{
//	ArrayObject *array;
//}ArrayRef;
//#define dereference(ref) (ref).array
ArrayObject *Array_New(size_t size, size_t item_size);

void Array_Dealloc(ArrayObject *self);

//void Array_Print(ArrayObject *self, FILE *out);

void Array_Clear(ArrayObject *self);

void Array_Lock(ArrayObject *self);

void Array_Unlock(ArrayObject *self);

void Array_Append(ArrayObject *self, void *item);

size_t Array_Find(ArrayObject *self, void *item, size_t start_index);

size_t Array_Count(ArrayObject *self, void *item);

void Array_Remove(ArrayObject *self, size_t index);

void Array_RemoveAll(ArrayObject *self, void *item);

void Array_SetItem(ArrayObject *self, size_t index, void *item);

void *Array_GetItem(ArrayObject *self, size_t index);

ArrayObject *Array_Copy(ArrayObject *self);

void Array_Swap(ArrayObject *self, ArrayObject *other);

#endif //R5RS_ARRAYOBJECT_H
