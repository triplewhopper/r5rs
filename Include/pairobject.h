#ifndef R5RS_PAIR_OBJECT_H
#define R5RS_PAIR_OBJECT_H

#include "object.h"

struct pair_object {
	Object ob_base;
	Object *car;
	Object *cdr;
	int is_list;
};

extern TypeObject Pair_Type;
extern PairObject empty_list;
#define EMPTY_LIST AS_OBJECT(&empty_list)
#define IS_NULL(obj) (AS_OBJECT(obj) == EMPTY_LIST)
#define IS_NOT_NULL(obj) (AS_OBJECT(obj) != EMPTY_LIST)
#define IS_LIST(obj) (IS_NULL(obj) || IS_TYPE(obj, Pair_Type) && Pair_IsList(AS_PAIR(obj)))
#define CAR(obj) AS_OBJECT(AS_PAIR(obj)->car)
#define CDR(obj) AS_OBJECT(AS_PAIR(obj)->cdr)

#define CONS(obj1, obj2) AS_OBJECT(Pair_New(AS_OBJECT(obj1), AS_OBJECT(obj2)))

PairObject *Pair_New(Object *car, Object *cdr);

//void Pair_Init(PairObject *, Object *, Object *);

void Pair_Print(PairObject *, FILE *);

void Pair_Repr(PairObject *, FILE *);

void Pair_Dealloc(PairObject *);

Object *Pair_GetCar(PairObject *);

Object *Pair_GetCdr(PairObject *);

int Pair_IsList(PairObject *);

size_t Pair_ListLength(Object *self);

int Pair_Equal(PairObject *, PairObject *);

void EmptyList_Print(PairObject *, FILE *);

void Pair_Eval(PairObject *self, VirtualMachineObject *vm);

#endif //R5RS_PAIR_OBJECT_H
