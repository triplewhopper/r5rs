#include "../Include/object.h"
ScmTypeObject *Scm_TYPE(ScmObject *op){

}
void Scm_DECREF(ScmObject *op){
	if (--op->ob_refcnt==0){
		Scm_Dealloc(op);
	}
}
void Scm_Dealloc(ScmObject *op){
	ScmTypeObject *type =
}