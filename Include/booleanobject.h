#ifndef _BOOLEAN_H_
#define _BOOLEAN_H_
#include "typeobject.h"

typedef struct Boolean{
	enum boolean_value_t
	{
		_true, _false
	}_value;
}Boolean;

int boolean_value_of(SObject *self){
	
}
#endif