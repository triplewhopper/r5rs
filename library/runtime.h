#ifndef R5RS_RUNTIME_H
#define R5RS_RUNTIME_H

#include "../Include/typedefs.h"


TypeObject *Super(Object *);

ChainMap *LoadBuiltinFuncs();

void AssertType(Object *, TypeObject *);


Object *is_pair(size_t, Object *[]);

Object *is_null(size_t argc, Object *argv[]);

Object *is_list(size_t argc, Object *argv[]);

Object *is_string(size_t argc, Object *argv[]);

Object *is_char(size_t argc, Object *argv[]);

Object *is_boolean(size_t argc, Object *argv[]);

Object *is_symbol(size_t argc, Object *argv[]);

Object *is_procedure(size_t argc, Object *argv[]);

//extern Object *IsExact(Object *);
//
//extern Object *IsInexact(Object *);


Object *car(size_t argc, Object *argv[]);

Object *cdr(size_t argc, Object *argv[]);

Object *cons(size_t argc, Object *argv[]);

Object *eq(size_t argc, Object *argv[]);

Object *eqv(size_t argc, Object *argv[]);

Object *equal(size_t argc, Object *argv[]);

Object *list_length(size_t argc, Object *argv[]);

Object *list_ref(size_t argc, Object *argv[]);

Object *display(size_t argc, Object *argv[]);

Object *newline(size_t argc, Object *argv[]);

Object *num_add(size_t argc, Object *argv[]);

Object *num_sub(size_t argc, Object *argv[]);

Object *num_mul(size_t argc, Object *argv[]);

Object *num_div(size_t argc, Object *argv[]);

Object *num_pow(size_t argc, Object *argv[]);

Object *num_rem(size_t argc, Object *argv[]);

Object *num_lt(size_t argc, Object *argv[]);

Object *num_le(size_t argc, Object *argv[]);

Object *num_eq(size_t argc, Object *argv[]);

Object *num_gt(size_t argc, Object *argv[]);

Object *num_ge(size_t argc, Object *argv[]);

Object *logical_and(size_t argc, Object *argv[]);

Object *logical_or(size_t argc, Object *argv[]);

Object *logical_not(size_t argc, Object *argv[]);

int perror_format(const char *format, ...);

#endif //R5RS_RUNTIME_H
