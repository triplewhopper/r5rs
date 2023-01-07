#ifndef R5RS_PROCEDURE_OBJECT_H
#define R5RS_PROCEDURE_OBJECT_H

#include "object.h"
#include "vmobject.h"

struct procedure_object {
	Object ob_base;
	Object *formals; // three cases
	CodeObject *code;
	StringObject *c_func_name;
	CFunction *c_function;
	ChainMap *lexical_scope;  // weak ref
	// exactly only one of lst and c_function is non-NULL.
	// lst==NULL && c_function!=NULL or vise versa.
};
extern TypeObject Procedure_Type;

ProcedureObject *Procedure_New(Object *, CodeObject *, ChainMap *);

ProcedureObject *Procedure_FromCFunction(StringObject *, CFunction);

void Procedure_Dealloc(ProcedureObject *self);

void Procedure_Traverse(ProcedureObject *self, visit_proc visit, void *arg);

int Procedure_FormalsCheckValid(Object *formals);

int Procedure_FormalsCheckUnique(Object *formals);

void Procedure_Search(ProcedureObject *self, Object *target, ArrayObject *res);

void Procedure_Print(ProcedureObject *, FILE *);

#endif //R5RS_PROCEDURE_OBJECT_H
