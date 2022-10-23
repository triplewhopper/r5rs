#ifndef _OBJ_H_
#define _OBJ_H_

#include <stdlib.h>
#include <stdio.h>
#include "typedefs.h"

struct scm_type_object {
	ScmVarObject ob_base;
	const char *const tp_name;

	ScmObject *(*const alloc)();

	void (*print)(ScmObject *, FILE *);

	void (*const init)(ScmObject *const);

	void (*const dealloc)(ScmObject *const);

	ScmObject *tp_dict;
	ScmTypeObject *tp_base;
};

extern ScmTypeObject ScmType_Type;

extern ScmTypeObject ScmBaseObject_Type;

void assert_type(ScmObject *obj, ScmTypeObject *obj_type) {
	if (obj->ob_type != obj_type) {
		char buf[100];
		sprintf(buf, "TypeError: expected type %s, got %s instead.", obj->ob_type->tp_name, obj_type->tp_name);
		perror(buf);
		exit(EXIT_FAILURE);
	}
}

#define CAST(ptr, T) ((T)(ptr))

#define INC_REF(obj) do {(obj)->ref_cnt++;}while(0)

#define DEC_REF(obj) do {assert((obj)->ref_cnt>0);if(--(obj)->ref_cnt==0){}} while(0)

void *my_malloc(size_t n_bytes) {
	void *res = malloc(n_bytes);
	if (res == NULL) {
		perror("malloc failed");
		exit(EXIT_FAILURE);
	} else {
		return res;
	}
}

void *my_realloc(void *original, size_t n_bytes) {
	void *tmp = realloc(original, n_bytes);
	if (tmp == NULL) {
		perror("realloc failed");
		exit(EXIT_FAILURE);
	} else {
		free(original);
		return tmp;
	}
}

#endif