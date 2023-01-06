#include <stdlib.h>
#include "../Include/typeobject.h"
#include "../Include/pairobject.h"
#include "../Include/chainmapobject.h"
#include "../Include/symbolobject.h"
#include "../Include/stringobject.h"
#include "../Include/procedureobject.h"
#include "../Include/frameobject.h"
#include "../Include/codeobject.h"

extern StringObject *to_string(Object *);

TypeObject Frame_Type = {
		.tp_name="frame",
		.tp_basicsize=sizeof(FrameObject),
		.tp_itemsize=0,
		.tp_print=(print_proc) Frame_Print,
		.tp_repr=(print_proc) NULL,
		.tp_dealloc=(dealloc_proc) Frame_Dealloc,
		.tp_flags=TPFLAGS_HAVE_GC,
		.tp_traverse=(traverse_proc) Frame_Traverse,
		.tp_search=(search_proc) Frame_Search
};

FrameObject *Frame_New(FrameObject *prev, ProcedureObject *func) {
	FrameObject *res = CAST(FrameObject *, TypeGenericAlloc(&Frame_Type, 0));
	res->fr_prev = XNEW_REF(prev);
	res->fr_env = ChainMap_NewChild(func->lexical_scope, NULL);
	res->fr_code = NEW_REF(func->code);
	res->pc = 0;
	gc_track(AS_OBJECT(res));
	return res;
}

FrameObject *Frame_Global(ChainMap *globals, CodeObject *code) {
	FrameObject *res = CAST(FrameObject *, TypeGenericAlloc(&Frame_Type, 0));
	res->fr_prev = NULL;
	res->fr_env = NEW_REF(globals);
	res->fr_code = NEW_REF(code);
	res->pc = 0;
	gc_track(AS_OBJECT(res));
	return res;

}

void Frame_Print(FrameObject *self, FILE *f) {
	int depth = 0;
	for (FrameObject *t = self; t; t = t->fr_prev) {
		fprintf(f, "depth=%d, in ", depth++);
		if (t->fr_code->co_name) {
			PRINT(t->fr_code->co_name, f);
		} else fprintf(f, "code #%zu", t->fr_code->co_id);
		fprintf(f, ": %zu/%d", t->pc, SIZE(t->fr_code));
		if (t->fr_prev == NULL)
			fprintf(f, ", global, ");
		else
			fprintf(f, ", local, ");

		fprintf(f, "env=");
		PRINT(t->fr_env->maps[0], f);

		fprintf(f, ":\n");
	}
	fprintf(f, "\n");
}

void Frame_Dealloc(FrameObject *self) {
	gc_untrack(AS_OBJECT(self));
	CLEAR(self->fr_prev);
	CLEAR(self->fr_env);
	CLEAR(self->fr_code);
}

void Frame_Traverse(FrameObject *self, visit_proc visit, void *arg) {
	if (self->fr_prev != NULL) VISIT(self->fr_prev, arg);
	VISIT(self->fr_env, arg);
	if (self->fr_code != NULL) VISIT(self->fr_code, arg);
}

void Frame_Search(FrameObject *self, Object *target, ArrayObject *res) {
	APPEND_PARENT(target, self, self->fr_prev);
	APPEND_PARENT(target, self, self->fr_env);
	APPEND_PARENT(target, self, self->fr_code);
	SEARCH(self->fr_prev, target, res);
	SEARCH(self->fr_env, target, res);
	SEARCH(self->fr_code, target, res);
}
