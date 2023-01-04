#include "../Include/object.h"
#include "../Include/typeobject.h"

void Type_SetBaseClass(TypeObject *t, TypeObject *base) {
#define fill_method(member) if(t->member==NULL) { t->member = base->member;}
	assert(t != base);
	assert(t != &BaseObject_Type);
	assert(t->tp_base == NULL);
	t->tp_base = base; // TODO: remember to change to ptr_set if decide to add reference counting to type objects.
	if (t->tp_repr == NULL && t->tp_print != NULL) t->tp_repr = t->tp_print;
	fill_method(tp_alloc);
//	fill_method(tp_dealloc);
//	fill_method(tp_print);
//	fill_method(tp_repr);
	fill_method(tp_cmp);
//	fill_method(tp_eval);

#undef fill_method
}

int Type_IsSubClass(TypeObject *t1, TypeObject *t2) {
	for (; t1; t1 = t1->tp_base) {
		if (t1 == t2) {
			return 1;
		}
	}
	return 0;
}

