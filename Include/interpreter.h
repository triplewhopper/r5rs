#ifndef R5RS_INTERPRETER_H
#define R5RS_INTERPRETER_H

#include "object.h"

struct g_symbols {
	Object *cond;
	Object *cond_case;
	Object *define;
	Object *cond_else;
	Object *cond_arrow;
	Object *if_expr;
	Object *lambda;
	Object *let;
	Object *let_star;
	Object *set;
	Object *quote;
};

struct g_symbols global_symbols;

typedef struct r5rs R5RS;

struct r5rs {
	struct g_symbols *keywords;
	ChainMap *globals;
	ChainMap *builtins;
};

R5RS *R5RS_New();

void R5RS_SetGlobalItem(R5RS *, SymbolObject *key, Object *value);

Object *R5RS_GetGlobalItem(R5RS *, SymbolObject *key);

void global_symbols_init();

void global_symbols_finalize();

void R5RS_Dealloc(R5RS *self);

#endif //R5RS_INTERPRETER_H
