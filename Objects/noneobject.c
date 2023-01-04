#include "../Include/typeobject.h"
#include "../Include/noneobject.h"


TypeObject None_Type = {
		.tp_name="None",
		.tp_basicsize=sizeof(NoneObject),
		.tp_itemsize=0,
		.tp_print=(print_proc) None_Print,
		.tp_dealloc=(dealloc_proc) None_Dealloc
};
NoneObject None = {
		{1, &None_Type}
};

void None_Dealloc(NoneObject *self) {
	assert(self == &None);
	fprintf(stderr, "in None_Dealloc(): None cannot be deallocated. Check your reference counting modification.\n");
	exit(EXIT_FAILURE);
}

void None_Print(Object *self, FILE *out) {
	assert(self == OBJ_NONE);
	fprintf(out, "#<unspecified>");
}


