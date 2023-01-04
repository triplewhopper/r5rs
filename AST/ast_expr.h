#ifndef R5RS_AST_EXPR_H
#define R5RS_AST_EXPR_H

#include "../Include/typedefs.h"
#include "../Include/booleanobject.h"
#include "../Include/pairobject.h"
#include "../Include/stringobject.h"
#include "../Include/symbolobject.h"
#include "../Include/noneobject.h"
#include "../Include/dictobject.h"
#include "../library/runtime.h"

void bad_form(const char *err_msg) {
	perror(err_msg);
	exit(EXIT_FAILURE);
}

typedef Object *(sematic_action)(Object *, DictObject *);

Object *Quote_Evaluate(Object *datum, DictObject *context);

Object *Lambda_Evaluate(Object *datum, DictObject *context);

Object *If_Evaluate(Object *datum, DictObject *context);

Object *Define_Evaluate(Object *cdr, DictObject *context);

Object *Expr_Evaluate(Object *datum, DictObject *context) {
	if (IsInstance(datum, &Symbol_Type)) {
		return Dict_GetItem(context, CAST(StringObject*, datum));
	} else if (Pair_IsList(datum)) {
		Object *head = car(datum);
		if (IsInstance(head, &Symbol_Type)) {
			const char *expression_keywords[] = {"quote", "lambda", "if", NULL};
			sematic_action *actions[] = {Quote_Evaluate, Lambda_Evaluate, Define_Evaluate};
			for(size_t i = 0; expression_keywords[i];++i){
				if(String_EqualCStr(CAST(StringObject*, head), expression_keywords[i])){
					return actions[i](datum, context);
				}
			}

		}
	} else {
		return datum;
	}
}

Object *Quote_Evaluate(Object *datum, DictObject *context) {
	INCREF(datum);
	return datum;
}

Object *Lambda_Evaluate(Object *datum, DictObject *context) {

}

// (if condition b1 b2)
// (if condition b1)
Object *If_Evaluate(Object *datum, DictObject *context) {
	Object *condition = car(datum);
	Object *branches = cdr(datum);
	Object *res_condition = Expr_Evaluate(condition, context);
	switch (List_Length(branches)) {
		case 1:
			if (IsFalse(res_condition)) {
				DECREF(res_condition);
				INCREF(&None);
				return Object_CAST(&None);
			} else {
				DECREF(res_condition);
				return Expr_Evaluate(car(branches), context);
			}
		case 2:
			if (IsFalse(res_condition)) {
				Object *t = car(cdr(branches));
				DECREF(res_condition);
				return Expr_Evaluate(t, context);
			} else {
				DECREF(res_condition);
				return Expr_Evaluate(car(branches), context);
			}
		default:
			DECREF(res_condition);
			perror("syntax error in if");
			exit(EXIT_FAILURE);
	}


}

Object *Define_Evaluate(Object *cdr, DictObject *context) {

}

#endif //R5RS_AST_EXPR_H
