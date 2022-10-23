#ifndef _PAIR_OBJECT_H_
#define _PAIR_OBJECT_H_
#include "typeobject.h"

typedef struct scm_pair{
	ScmObject_HEAD
	ScmObject *car;
	ScmObject *cdr;
}ScmPairObject;

ScmObject* pair_alloc();
void pair_init(ScmObject* self);
void pair_dealloc(ScmObject* self);

const ScmTypeObject pair_t={
	.name="scm_pair",
	.alloc=pair_alloc, 
	.init=pair_init, 
	.dealloc=pair_dealloc
};

ScmObject* pair_alloc(){
	ScmObject *obj = my_malloc(sizeof(ScmObject));
	scm_pair *p = my_malloc(sizeof(scm_pair));
	*obj = (ScmObject){._type= &pair_t, .ref_cnt=0, .is_mutable=1, (void*)p};
	return obj;
}
void pair_init(ScmObject* self){
	ASSERT_TYPE(self, pair_t);
	scm_pair *p = CAST(self->_obj, scm_pair*);
	p->car = NULL;
	p->cdr = NULL;
}
void pair_dealloc(ScmObject *self){
	ASSERT_TYPE(self, Pair);
	
}
ScmObject* pair_car(ScmObject *self){
	ASSERT_TYPE(self, Pair);
	return (CAST(self->_obj, scm_pair*))->car;
}

ScmObject* pair_cdr(ScmObject *self){
	ASSERT_TYPE(self, Pair);
	return (CAST(self->_obj, scm_pair*))->cdr;
}
size_t pair_hash(ScmObject *self){
	ASSERT_TYPE(self, Pair);

}
#endif
