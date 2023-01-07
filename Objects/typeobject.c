#include "../Include/object.h"
#include "../Include/typeobject.h"

static void Type_SetBaseClass(TypeObject *t, TypeObject *base) {
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

#undef fill_method
}

void Type_InitTypeObjects() {
	Type_SetBaseClass(&Boolean_Type, &BaseObject_Type);
	Type_SetBaseClass(&Long_Type, &BaseObject_Type);
	Type_SetBaseClass(&Float_Type, &BaseObject_Type);
	Type_SetBaseClass(&Fraction_Type, &BaseObject_Type);
	Type_SetBaseClass(&Complex_Type, &BaseObject_Type);
	Type_SetBaseClass(&Pair_Type, &BaseObject_Type);
	Type_SetBaseClass(&Char_Type, &BaseObject_Type);
	Type_SetBaseClass(&String_Type, &BaseObject_Type);
	Type_SetBaseClass(&Symbol_Type, &BaseObject_Type);
	Type_SetBaseClass(&None_Type, &BaseObject_Type);
	Type_SetBaseClass(&Vector_Type, &BaseObject_Type);
	Type_SetBaseClass(&Procedure_Type, &BaseObject_Type);

	Type_SetBaseClass(&Token_Type, &BaseObject_Type);
	Type_SetBaseClass(&Dict_Type, &BaseObject_Type);
	Type_SetBaseClass(&Array_Type, &BaseObject_Type);
	Type_SetBaseClass(&ChainMap_Type, &BaseObject_Type);

	Type_SetBaseClass(&Lexer_Type, &BaseObject_Type);
	Type_SetBaseClass(&Code_Type, &BaseObject_Type);
	Type_SetBaseClass(&Frame_Type, &BaseObject_Type);
	Type_SetBaseClass(&VirtualMachine_Type, &BaseObject_Type);
}

