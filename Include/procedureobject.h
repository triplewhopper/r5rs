#ifndef R5RS_PROCEDURE_OBJECT_H
#define R5RS_PROCEDURE_OBJECT_H

#include "object.h"
#include "vmobject.h"

struct procedure_object {
	Object ob_base;
	Object *formals;
	CodeObject *code;
	StringObject *c_func_name;
	CFunction *c_function;
	ChainMap *lexical_scope;
	// exactly only one of lst and c_function is non-NULL.
	// lst==NULL && c_function!=NULL or vise versa.
};
extern TypeObject Procedure_Type;

ProcedureObject *Procedure_New(Object *, CodeObject *, ChainMap *);

ProcedureObject *Procedure_FromCFunction(StringObject *, CFunction);

void Procedure_Dealloc(ProcedureObject *self);

int Procedure_FormalsCheckValid(Object *formals);

int Procedure_FormalsCheckUnique(Object *formals);

void Procedure_Print(ProcedureObject *, FILE *);
//int Procedure_NArgs(ProcedureObject *);

DictObject *locals();

#endif //R5RS_PROCEDURE_OBJECT_H
