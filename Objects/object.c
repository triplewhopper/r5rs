#include "../Include/object.h"
#include "../Include/typeobject.h"
#include "../Include/vmobject.h"
#include "../Include/codeobject.h"
#include "../Include/numberobject.h"
#include "../Include/arrayobject.h"
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
Object *live_objects[OBJECT_MAX_NUM];
static int n_objects;
static int n_live_objs;
#endif

Object *TypeGenericAlloc(TypeObject *type, size_t size) {
	Object *res = NULL;
	if (type->tp_flags & TPFLAGS_HAVE_GC) {
		res = malloc(sizeof(GC_Head) + type->tp_basicsize + size * type->tp_itemsize);
		res = AS_OBJECT((GC_Head *) res + 1);
	} else {
		res = malloc(type->tp_basicsize + size * type->tp_itemsize);
	}

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
	res->searched = 0;
#endif
	res->ob_refcnt = 1;
	res->ob_type = type;
	res->finalized = 0;

	if (type->tp_itemsize > 0) {
		SIZE(res) = (int) size;
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
	if (TYPE(op)->tp_flags & TPFLAGS_HAVE_GC) {
		free((GC_Head *) op - 1);
	} else {
		free(op);
	}
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

ChainMap *get_live_chainmapobj() {
	for (size_t i = 0; i < n_objects; ++i) {
		if (live_objects[i] != NULL && IS_TYPE(live_objects[i], ChainMap_Type)) {
			return CAST(ChainMap *, live_objects[i]);
		}
	}
	return NULL;
}

#endif

ArrayObject *gc_objs;
ArrayObject *gc_unreachable;
int gc_is_collecting;

void DecRef(Object *op) {
	assert(op->finalized == 1 || op->ob_refcnt > 0);
#ifdef FLAG_TRACK_ALL_OBJS
	assert(op->obj_index == 0 || live_objects[op->obj_index] != NULL);
#endif
	if (!op->finalized && --op->ob_refcnt == 0) {
		op->finalized = 1;
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

void gc_init() {
	assert(gc_objs == NULL && gc_unreachable == NULL || gc_objs != NULL && gc_unreachable != NULL);
	if (gc_objs == NULL) {
		gc_objs = Array_New(0, sizeof(Object *));
		gc_unreachable = Array_New(0, sizeof(Object *));
	} else {
		Array_Clear(gc_objs);
		Array_Clear(gc_unreachable);
	}

}

void gc_finalize() {
	assert(IS_OWNED(gc_objs));
	assert(IS_OWNED(gc_unreachable));
	if (SIZE(gc_objs) == 0) CLEAR(gc_objs);
	if (SIZE(gc_unreachable) == 0) CLEAR(gc_unreachable);
}

void gc_track(Object *obj) {
	if (!Array_Count(gc_objs, &obj)) {
//		for (size_t k = 0; k < SIZE(gc_objs); ++k) {
//			assert(Array_At(Object * , gc_objs, k)->obj_index > 0);
//		}
		Array_Append(gc_objs, &obj);
//		for (size_t k = 0; k < SIZE(gc_objs); ++k) {
//			assert(Array_At(Object * , gc_objs, k)->obj_index > 0);
//		}
	}
}

void gc_untrack(Object *obj) {
	size_t i = Array_Find(gc_objs, &obj, 0);
	if (i < SIZE(gc_objs)) {
		Array_Remove(gc_objs, i);
	} else {
		assert(gc_is_collecting);
		i = Array_Find(gc_unreachable, &obj, 0);
		assert(i < SIZE(gc_unreachable));
		Array_Remove(gc_unreachable, i);
	}
//		assert(!gc_is_collecting);
//	for (size_t k = 0; k < SIZE(gc_objs); ++k) {
//		assert(Array_At(Object * , gc_objs, k)->obj_index > 0);
//	}
//	for (size_t k = 0; k < SIZE(gc_objs); ++k) {
//		assert(Array_At(Object * , gc_objs, k)->obj_index > 0);
//	}
//	} else {
//		assert(gc_is_collecting);
//		i = Array_Find(gc_unreachable, &obj, 0);
//		assert(i < SIZE(gc_unreachable));
//		Array_SetItem(gc_unreachable, i, (Object *[]) {NULL});
//	}
}

#define GC_HEADER(obj) ((GC_Head *) (obj) - 1)

int gc_visit_proc1(Object *obj, void *arg) {
	if (TYPE(obj)->tp_flags & TPFLAGS_HAVE_GC) {
		assert(Array_Find(gc_objs, &obj, 0) < SIZE(gc_objs));
		assert(GC_HEADER(obj)->gc_ref > 0);
		GC_HEADER(obj)->gc_ref--;
//		if (GC_HEADER(obj)->visited == 0) {
//			GC_HEADER(obj)->visited = 1;
//		}
	}
	return 0;
}

int gc_visit_proc2(Object *obj, void *arg) {
	if (TYPE(obj)->tp_flags & TPFLAGS_HAVE_GC && GC_HEADER(obj)->visited == 0) {
//		GC_HEADER(obj)->visited = 1;
		if (GC_HEADER(obj)->gc_ref == 0) {
			size_t i = Array_Find(gc_unreachable, &obj, 0);
			assert(i < SIZE(gc_unreachable));
			Array_Remove(gc_unreachable, i);
			assert(Array_Find(arg, &obj, 0) == SIZE(arg));
			Array_Append(arg, &obj);
			GC_HEADER(obj)->gc_ref = 1;
		} else {
			assert(Array_Find(arg, &obj, 0) < SIZE(arg));
			assert(Array_Find(gc_unreachable, &obj, 0) == SIZE(gc_unreachable));
		}
//		TRAVERSE(obj, gc_visit_proc2, arg);
	}
	return 0;
}

int gc_visit_proc3(Object *obj, void *arg) {
	if (TYPE(obj)->tp_flags & TPFLAGS_HAVE_GC) {
		if (GC_HEADER(obj)->gc_ref == 0) { ;
		} else {
			DECREF(obj);
		}
	} else {
		DECREF(obj);
	}
	return 0;
}

#ifdef FLAG_TRACK_ALL_OBJS

void get_parents(Object *obj) {
	for (size_t i = 0; i < n_objects; ++i) {
		if (live_objects[i] != NULL) live_objects[i]->searched = 0;
	}
	ArrayObject *parents = Array_New(0, sizeof(Object *));
	for (size_t i = 0; i < n_objects; ++i) {
		SEARCH(live_objects[i], obj, parents);
	}
	DECREF(parents);
}

#endif

void gc_collect() {
	assert(!gc_is_collecting);
	gc_is_collecting = 1;
	Array_Clear(gc_unreachable);
	Array_Lock(gc_objs);
	for (size_t i = 0; i < SIZE(gc_objs); ++i) {
		Object *obj = Array_At(Object *, gc_objs, i);
		assert(TYPE(obj)->tp_flags & TPFLAGS_HAVE_GC);
#ifdef FLAG_TRACK_ALL_OBJS
		assert(live_objects[obj->obj_index] != NULL);
#endif
		GC_HEADER(obj)->gc_ref = REFCNT(obj);
		GC_HEADER(obj)->visited = 0;
	}
	for (size_t i = 0; i < SIZE(gc_objs); ++i) {
		Object *obj = Array_At(Object *, gc_objs, i);
		TRAVERSE(obj, gc_visit_proc1, NULL);
	}

	ArrayObject *survivals = Array_New(0, sizeof(Object *));
	for (size_t i = 0; i < SIZE(gc_objs); ++i) {
		Object *obj = Array_At(Object *, gc_objs, i);
		GC_HEADER(obj)->visited = 0;
		if (GC_HEADER(obj)->gc_ref == 0) {
			Array_Append(gc_unreachable, &obj);
		} else {
			Array_Append(survivals, &obj);
		}
	}
	assert(SIZE(survivals) + SIZE(gc_unreachable) == SIZE(gc_objs));
	Array_Unlock(gc_objs);
	Array_Clear(gc_objs);
//	for (size_t i = 0; i < SIZE(survivals); ++i) {
//		Object *obj = Array_At(Object *, survivals, i);
//		if (GC_HEADER(obj)->gc_ref > 0) {
//			Array_Append(gc_objs, &obj);
//		}
//	}
	for (size_t i = 0; i < SIZE(survivals); ++i) {
		Object *obj = Array_At(Object *, survivals, i);
		if (GC_HEADER(obj)->gc_ref > 0) {
			TRAVERSE(obj, gc_visit_proc2, survivals);
		}
	}
	Array_Swap(gc_objs, survivals);
//	get_parents(*(Object **) Array_GetItem(survivals, 0));
	Array_Lock(gc_unreachable);
	for (size_t i = 0; i < SIZE(gc_unreachable); ++i) {
		Object *obj = Array_At(Object *, gc_unreachable, i);
		assert(REFCNT(obj) > 0);
		TRAVERSE(obj, gc_visit_proc3, NULL);
	}
	for (size_t i = 0; i < SIZE(gc_unreachable); ++i) {
		Object *obj = Array_At(Object *, gc_unreachable, i);
		assert(REFCNT(obj) > 0);
		Object_Dealloc(obj);
	}
	Array_Unlock(gc_unreachable);
	Array_Clear(gc_unreachable);
	gc_is_collecting = 0;
	CLEAR(survivals);
}

int gc_is_tracked(Object *obj) {
	return TYPE(obj)->tp_flags & TPFLAGS_HAVE_GC;
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