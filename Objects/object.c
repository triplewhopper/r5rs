#include "../Include/object.h"
#include "../Include/typeobject.h"
#include "../Include/vmobject.h"
#include "../Include/codeobject.h"
#include "../Include/numberobject.h"
#include <assert.h>


CompareMethods Object_Compare = {
		.cmp_eq = Object_Eq,
		.cmp_eqv = Object_Eq,
		.cmp_equal = Object_Eq
};
TypeObject BaseObject_Type = {
		.tp_name="object",
		.tp_basicsize=sizeof(Object),
		.tp_itemsize=0,
		.tp_alloc=TypeGenericAlloc,
		.tp_print=NULL,
		.tp_dealloc=Object_Dealloc,
		.tp_base=NULL,
		.tp_cmp = &Object_Compare,
//		.tp_eval = Eval_Default
};
#ifdef FLAG_TRACK_ALL_OBJS
static Object *live_objects[100000];
static int n_objects;
static int n_live_objs;
#endif

Object *TypeGenericAlloc(TypeObject *type, size_t size) {
	if(type->tp_flags & TPFLAGS_HAVE_GC){
		gc_track(res);
	}
	Object *res = malloc(type->tp_basicsize + size * type->tp_itemsize);
	if (res == NULL) {
		char msg[100];
		snprintf(msg, 99, "No memory for %s object", type->tp_name);
		perror(msg);
		exit(EXIT_FAILURE);
	}
#ifdef FLAG_TRACK_ALL_OBJS
	n_objects++;
	n_live_objs++;
	live_objects[n_objects] = res;
	res->obj_index = n_objects;
#endif
	res->ob_refcnt = 1;
	res->ob_type = type;

	if (type->tp_itemsize > 0) {
		((VarObject *) res)->ob_size = (int) size;
	}
	return res;
}

void Object_Dealloc(Object *op) {
#ifdef FLAG_TRACK_ALL_OBJS
	assert(op->obj_index);
	assert(op == live_objects[op->obj_index]);
	live_objects[op->obj_index] = NULL;
	n_live_objs--;
#endif
	free(op);
}

int RefCnt(Object *op) {
	return op->ob_refcnt;
}

void IncRef(Object *op) {
	assert(op->ob_refcnt);
	op->ob_refcnt++;
}

void XIncRef(Object *op) {
	if (op) IncRef(op);
};

#ifdef FLAG_TRACK_ALL_OBJS

static int find(Object *obj) {
	assert(obj);
	for (int i = 0; i < n_objects; ++i) {
		if (live_objects[i] == obj) return i;
	}
	return -1;
}

size_t count_live_objs(TypeObject *type) {
	size_t ans = 0;
	for (size_t i = 0; i < n_objects; ++i) {
		if (live_objects[i] != NULL && TYPE(live_objects[i]) == type) {
			assert(REFCNT(live_objects[i]) > 0);
			ans++;
		}
	}
	return ans;
}

CodeObject *get_live_codeobj() {
	for (size_t i = 0; i < n_objects; ++i) {
		if (live_objects[i] != NULL && IS_TYPE(live_objects[i], Code_Type)) {
			return CAST(CodeObject *, live_objects[i]);
		}
	}
	return NULL;
}

#endif


void DecRef(Object *op) {
	assert(op->ob_refcnt > 0);
	if (--op->ob_refcnt == 0) {
		TypeObject *t = TYPE(op);
		int flag = 0;
		for (; t; t = t->tp_base) {
			if (t->tp_dealloc) flag = 1, t->tp_dealloc(op); else flag = 0;
		}
		assert(flag);
	}
}

void XDecRef(Object *op) {
	if (op) DecRef(op);
}

//// (x: &mut &obj, v: &obj) -> i32
//int ptr_set(Object **x, Object *v) {
//	assert(x != NULL);
//	assert(v != NULL);
//	if (*x != v) {
//		INCREF(v);
//		XDECREF(*x);
//		*x = v;
//		return 1;
//	} else {
//		return 0;
//	}
//}

void Eval_Default(Object *obj, VirtualMachineObject *vm) {
	assert(0);
//	VirtualMachine_Push(vm, obj);
}

int Object_Eq(Object *obj1, Object *obj2) {
	return obj1 == obj2;
}

Object *NewRef(Object *obj) {
	INCREF(obj);
	return obj;
}

Object *XNewRef(Object *obj) {
	XINCREF(obj);
	return obj;
}