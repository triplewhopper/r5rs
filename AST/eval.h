#ifndef R5RS_EVAL_H
#define R5RS_EVAL_H
#include "../Include/object.h"
struct env;
typedef struct env Env;
Object *eval(Env *env);
Object *eval_var(Env *env);
Object *eval_

#endif //R5RS_EVAL_H
