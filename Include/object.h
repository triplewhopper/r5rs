#ifndef R5RS_OBJECT_H
#define R5RS_OBJECT_H

#include "typedefs.h"

#define ScmVarObject_HEAD \
	ScmObject_HEAD \
	size_t ob_size;

#define ScmObject_HEAD_INIT(type) {1, type}

#define ScmVarObject_HEAD_INIT(type, size) \
    ScmObject_HEAD_INIT(type) size

#define ScmObject_VAR_HEAD ScmVarObject ob_base;

struct scm_object{
	i32 ob_refcnt;
	ScmTypeObject * ob_type;
};

typedef struct scm_var_object{
	ScmObject ob_base;
	i32 ob_size;
}ScmVarObject;

void Scm_DECREF(ScmObject *op);
void Scm_Dealloc(ScmObject *op);
ScmTypeObject *Scm_TYPE(ScmObject *op);
#endif //R5RS_OBJECT_H
