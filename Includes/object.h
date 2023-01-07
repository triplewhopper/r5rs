#ifndef R5RS_OBJECT_H
#define R5RS_OBJECT_H

#include <string.h>
#include "typedefs.h"

//#define FLAG_TRACK_ALL_OBJS
//#define OBJECT_MAX_NUM 100000
struct object {
	int ob_refcnt;
	TypeObject *ob_type;
	int finalized;
	int searched;
#ifdef FLAG_TRACK_ALL_OBJS
	int obj_index;
#endif
};

typedef struct gc_head {
	int gc_ref;
	int visited;
} GC_Head;

typedef struct scm_var_object {
	Object ob_base;
	int ob_size;
} VarObject;

extern TypeObject BaseObject_Type;

Object *TypeGenericAlloc(TypeObject *, size_t);

void Object_Dealloc(Object *);

int RefCnt(Object *);

void IncRef(Object *);

void DecRef(Object *);

void XIncRef(Object *op);

void XDecRef(Object *op);

int Object_Eq(Object *obj1, Object *obj2);

Object *NewRef(Object *);

Object *XNewRef(Object *obj);

void get_parents(Object *obj);

void gc_init();

void gc_finalize();

void gc_track(Object *);

void gc_untrack(Object *);

void gc_collect();

int gc_is_tracked(Object *);

#ifdef FLAG_TRACK_ALL_OBJS

size_t count_live_objs(TypeObject *type);

ChainMap *get_live_chainmapobj();

#endif


#define CAST(type, expr) ((type)(expr))
#define AS_OBJECT(obj) CAST(Object*, (obj))
#define TYPE(obj) CAST(TypeObject*, AS_OBJECT(obj)->ob_type)
#define IS_TYPE(obj, type_obj) (TYPE(obj)==&(type_obj))
#define VarObject_CAST(obj) CAST(VarObject*, obj)
#define AS_LONG(obj) CAST(LongObject*, obj)
#define AS_FLOAT(obj) CAST(FloatObject*, obj)
#define AS_FRAC(obj) CAST(FractionObject*, obj)
#define AS_COMPLEX(obj) CAST(ComplexObject*, obj)
#define AS_PAIR(obj) CAST(PairObject*, obj)
#define AS_SYMBOL(obj) CAST(SymbolObject*, obj)
#define AS_STRING(obj) CAST(StringObject*, obj)
#define AS_PROCEDURE(obj) CAST(ProcedureObject*, obj)
#define SIZE(obj) (VarObject_CAST(obj)->ob_size)
#define EQV(obj1, obj2) (TYPE(obj1)->tp_cmp->cmp_eqv(AS_OBJECT(obj1), AS_OBJECT(obj2)))

#define REFCNT(op) RefCnt(AS_OBJECT(op))
#define INCREF(op) IncRef(AS_OBJECT(op))
#define DECREF(op) DecRef(AS_OBJECT(op))
#define XINCREF(op) XIncRef(AS_OBJECT(op))
#define XDECREF(op) XDecRef(AS_OBJECT(op))

#define CLEAR(op) \
    do { \
        typeof(op)* _tmp_op_ptr = &(op); \
        typeof(op) _tmp_old_op = (*_tmp_op_ptr); \
        if (_tmp_old_op != NULL) { \
            *_tmp_op_ptr = NULL; \
            DECREF(_tmp_old_op); \
        } \
    } while (0)

#define IS_OWNED(ref) (REFCNT(ref) == 1)
#define IS_SHARED(ref) (REFCNT(ref) > 1)
#define NEW_REF(op) CAST(typeof(op), NewRef(AS_OBJECT(op)))
#define XNEW_REF(op) CAST(typeof(op), XNewRef(AS_OBJECT(op)))
#define WEAK_REF(op) CAST(typeof(op), op)
//#define PTR_SET(x, v) ptr_set((Object**)(x), (Object*)(v))
//#define MOVE(dest, src) move((Object**)&(dest), (Object**))
#define MOVE_SET(a, b, c) do { (a)=(b); (b)=(c); DECREF(a); } while(0)


#endif //R5RS_OBJECT_H
