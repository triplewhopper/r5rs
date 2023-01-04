#include "../Include/typeobject.h"
#include "../Include/booleanobject.h"
#include <assert.h>

TypeObject Boolean_Type = {
		.tp_name="boolean",
		.tp_basicsize=sizeof(BooleanObject),
		.tp_itemsize=0,
		.tp_print=(print_proc) Boolean_Print,
		.tp_dealloc=(dealloc_proc) Boolean_Dealloc,
};

BooleanObject Boolean_True = {
		{1, &Boolean_Type}
};
BooleanObject Boolean_False = {
		{1, &Boolean_Type}
};

void Boolean_Print(BooleanObject *self, FILE *out) {
	assert(self == &Boolean_False || self == &Boolean_True);
	if (self == &Boolean_True) {
		fprintf(out, "#t");
	} else {
		fprintf(out, "#f");
	}
}

void Boolean_Dealloc(BooleanObject *self) {
	assert(self == &Boolean_False || self == &Boolean_True);
	assert(0);
}

