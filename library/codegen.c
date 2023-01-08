#include <stdarg.h>
#include "object.h"
#include "noneobject.h"
#include "symbolobject.h"
#include "pairobject.h"
#include "codeobject.h"
#include "procedureobject.h"
#include "lexer.h"
#include "interpreter.h"
#include "parser.h"
#include "codegen.h"

// structural_match("(? ? ? . ?)", &x, &y, &z, &others)
static int structural_match_impl(Object *obj, Object *datum, va_list ap) {
	if (IS_TYPE(datum, Symbol_Type)) {
		if (Symbol_Eqv_CStr(AS_SYMBOL(datum), "?")) {
			*va_arg(ap, Object**) = obj;
			return 1;
		} else if (!EQV(obj, datum)) {
			return -1;
		}
		return 0;
	}
	if (IS_TYPE(datum, Pair_Type)) {
		if (IS_TYPE(obj, Pair_Type)) {
			int a = structural_match_impl(CAR(obj), CAR(datum), ap);
			if (a == -1) return -1;
			int b = structural_match_impl(CDR(obj), CDR(datum), ap);
			if (b == -1) return -1;
			return a + b;
		} else {
			return -1;
		}
	}
	return 0;
}

static int structural_match(Object *obj, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	LexerObject *lexer = Lexer_FromStream(fmemopen((char *) format, strlen(format), "r"), NULL);
	Object *datum = ParseDatum(lexer);
#ifdef __DEBUG__
	printf("in function %s, datum=", __FUNCTION__);
	PRINT(datum, stdout);
	putchar('\n');
#endif
	int res = structural_match_impl(obj, datum, ap);
	assert(ParseDatum(lexer) == NULL);
	DECREF(datum);
	DECREF(lexer);
	va_end(ap);
	return res;
}

//static void codegen_if(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope);
//
//static void codegen_define(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope);
//
//static void codegen_set(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope);
//
//static void codegen_lambda_body(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope);

static void codegen_impl(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope);

static void codegen_lambda_body(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope) {
	assert(IS_LIST(o));
	assert(IS_NOT_NULL(o));
	assert(define_flag);
	for (; IS_NOT_NULL(o); o = CDR(o)) {
		assert(IS_NOT_NULL(CAR(o)));
		if (IS_TYPE(CAR(o), Pair_Type) && !EQV(CAR(CAR(o)), global_symbols.define)) {
			define_flag = 0;
		}
		if (IS_NOT_NULL(CDR(o))) {
			codegen_impl(CAR(o), code, 0, define_flag, scope);
			NEW_INSTRUCTION(code, VM_POP);
		} else {
			codegen_impl(CAR(o), code, tail_flag, define_flag, scope);
		}
	}
}

static void codegen_define(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope) {
	assert(define_flag != 0);
	Object *var = NULL, *formals = NULL, *e = NULL;
	if (structural_match(o, "(define (? . ?) . ?)", &var, &formals, &e) == 3) {
		assert(IS_TYPE(var, Symbol_Type));
		assert(Procedure_FormalsCheckValid(formals));
		assert(Procedure_FormalsCheckUnique(formals));

		NEW_INSTRUCTION(code, VM_LOAD, NewRef(var));
		NEW_INSTRUCTION(code, VM_LOAD, NewRef(formals));

		CodeObject *code1 = Code_New(var);
		codegen_lambda_body(e, code1, 1, 1, scope);
		Code_Freeze(code1);

		NEW_INSTRUCTION(code, VM_LOAD, AS_OBJECT(code1));
		NEW_INSTRUCTION(code, VM_MAKE_FUNCTION);
		NEW_INSTRUCTION(code, VM_DEFINE);
		NEW_INSTRUCTION(code, VM_LOAD, NewRef(OBJ_NONE));
		return;
	} else if (structural_match(o, "(define ? ?)", &var, &e) == 2) {
		assert(IS_TYPE(var, Symbol_Type));

		NEW_INSTRUCTION(code, VM_LOAD, NewRef(var));
		codegen_impl(e, code, 1, 0, scope);
		NEW_INSTRUCTION(code, VM_DEFINE);
		NEW_INSTRUCTION(code, VM_LOAD, NewRef(OBJ_NONE));
		return;
	}
	assert(0);
}

void codegen_if(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope) {
	Object *test = NULL;
	Object *e1 = NULL, *e2 = NULL;
	if (structural_match(o, "(if ? ? ?)", &test, &e1, &e2) == 3) {
		codegen_impl(test, code, 0, 0, scope);
		const size_t tmp = SIZE(code);
		NEW_INSTRUCTION(code, VM_JUMP_IF_FALSE);

		codegen_impl(e1, code, tail_flag, 0, scope);
		const size_t tmp2 = SIZE(code);
		NEW_INSTRUCTION(code, VM_JUMP_FORWARD);

		codegen_impl(e2, code, tail_flag, 0, scope);

		Code_SetOpArg(code, tmp, tmp2 + 1 - tmp);
		Code_SetOpArg(code, tmp2, SIZE(code) - tmp2);

	} else if (structural_match(o, "(if ? ?)", &test, &e1) == 2) {
		codegen_impl(test, code, tail_flag, 0, scope);
		const size_t tmp = SIZE(code);
		NEW_INSTRUCTION(code, VM_JUMP_IF_FALSE);

		codegen_impl(e1, code, tail_flag, 0, scope);
		const size_t tmp2 = SIZE(code);
		NEW_INSTRUCTION(code, VM_JUMP_FORWARD);

		NEW_INSTRUCTION(code, VM_LOAD, NewRef(OBJ_NONE));

		Code_SetOpArg(code, tmp, tmp2 + 1 - tmp);
		Code_SetOpArg(code, tmp2, SIZE(code) - tmp2);

	} else {
		assert(0);
	}
}

static void codegen_set(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope) {
	Object *var = NULL, *e = NULL;
	assert(structural_match(o, "(set! ? ?)", &var, &e) == 2);
	assert(IS_TYPE(var, Symbol_Type));
	NEW_INSTRUCTION(code, VM_LOAD, NewRef(var));
	codegen_impl(e, code, 0, 0, scope);
	NEW_INSTRUCTION(code, VM_DEEP_STORE);
	NEW_INSTRUCTION(code, VM_LOAD, NewRef(OBJ_NONE));
}

static void codegen_let(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope) {
	Object *bindings = NULL, *name = NULL, *body = NULL, *b = NULL, *tmp;
	if (structural_match(o, "(let ? ? ?)", &name, &bindings, &body) == 3) {
		assert(IS_TYPE(name, Symbol_Type));
	} else {
		name = NULL;
		assert(structural_match(o, "(let ? ?)", &bindings, &body) == 2);
	}
	assert(IS_LIST(bindings));
	size_t n_bindings = Pair_ListLength(bindings);
	Object **_formals = calloc(n_bindings, sizeof(Object *));
	Object **_inits = calloc(n_bindings, sizeof(Object *));
	assert(_formals);
	assert(_inits);
	size_t i;
	for (i = 0, b = bindings; i < n_bindings; ++i, b = CDR(b)) {
		Object *var = NULL, *init = NULL;
		assert(structural_match(CAR(b), "(? ?)", &var, &init) == 2);
		assert(IS_TYPE(var, Symbol_Type));
		_formals[i] = var;
		_inits[i] = init;
	}
	Object *formals = NewRef(EMPTY_LIST);
	for (i = 1; i <= n_bindings; ++i) {
		MOVE_SET(tmp, formals, CONS(_formals[n_bindings - i], formals));
	}
	assert(Procedure_FormalsCheckValid(formals));
	assert(Procedure_FormalsCheckUnique(formals));
	NEW_INSTRUCTION(code, VM_LOAD, formals);
	CodeObject *code1 = NULL;
	if (name != NULL) {
		code1 = Code_New(name);
	} else {
		static size_t let_count = 0;
		char name_buf[20] = {0};
		sprintf(name_buf, "let$%zu", ++let_count);
		name = AS_OBJECT(Symbol_FromCStr(name_buf));
		code1 = Code_New(name);
		DECREF(name);
	}
	codegen_impl(body, code1, tail_flag, 0, NULL);
	Code_Freeze(code1);
	NEW_INSTRUCTION(code, VM_LOAD, AS_OBJECT(code1));
	NEW_INSTRUCTION(code, VM_MAKE_FUNCTION);
	for (i = 0; i < n_bindings; ++i) {
		codegen_impl(_inits[i], code, 0, 0, scope);
	}
	NEW_INSTRUCTION(code, VM_LOAD, NewRef(EMPTY_LIST));
	for (i = 0; i < n_bindings; ++i) {
		NEW_INSTRUCTION(code, VM_CONS);
	}
	NEW_INSTRUCTION(code, tail_flag ? VM_TAIL_CALL : VM_CALL);
	free(_formals);
	free(_inits);

}

static void codegen_cond(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope) {
	assert(EQV(global_symbols.cond, CAR(o)));
	Object *clauses = NULL;
	int has_else = 0;
	ArrayObject *jump_if_false_indices = Array_New(0, sizeof(size_t));
	ArrayObject *jump_forward_indices = Array_New(0, sizeof(size_t));
	ArrayObject *test_begin_indices = Array_New(0, sizeof(size_t));
//	NEW_INSTRUCTION(code, VM_LOAD, AS_OBJECT(String_FromCStr("cond")));
//	NEW_INSTRUCTION(code, VM_POP);
	for (clauses = CDR(o); IS_NOT_NULL(clauses); clauses = CDR(clauses)) {
		Object *expressions = CAR(clauses);
		assert(IS_LIST(expressions));
		size_t len = Pair_ListLength(expressions);
		assert(len >= 1);
		if (EQV(global_symbols.cond_else, CAR(expressions))) {
			assert(IS_NULL(CDR(clauses)));
			assert(len > 1);
			has_else = 1;
			Array_Append(test_begin_indices, (size_t[]) {SIZE(code)});
			for (expressions = CDR(expressions); IS_NOT_NULL(CDR(expressions)); expressions = CDR(expressions)) {
				codegen_impl(CAR(expressions), code, 0, 0, scope);
				NEW_INSTRUCTION(code, VM_POP);
			}
			assert(IS_NULL(CDR(expressions)));
			codegen_impl(CAR(expressions), code, tail_flag, 0, scope);

		} else {
			Object *test = CAR(expressions);
			expressions = CDR(expressions);

			Array_Append(test_begin_indices, (size_t[]) {SIZE(code)});
			codegen_impl(test, code, 0, 0, scope);
			if (len == 1) {
				NEW_INSTRUCTION(code, VM_COPY);
			}
			Array_Append(jump_if_false_indices, (size_t[]) {SIZE(code)});
			NEW_INSTRUCTION(code, VM_JUMP_IF_FALSE);
			if (len == 3 && EQV(global_symbols.cond_arrow, CAR(expressions))) {

				codegen_impl(CAR(CDR(expressions)), code, 0, 0, scope);
				NEW_INSTRUCTION(code, VM_SWAP);
				NEW_INSTRUCTION(code, VM_LOAD, NewRef(EMPTY_LIST));
				NEW_INSTRUCTION(code, VM_CONS);
				NEW_INSTRUCTION(code, tail_flag ? VM_TAIL_CALL : VM_CALL);

			} else if (len >= 2) {

				for (; IS_NOT_NULL(CDR(expressions)); expressions = CDR(expressions)) {
					codegen_impl(CAR(expressions), code, 0, 0, scope);
					NEW_INSTRUCTION(code, VM_POP);
				}
				assert(IS_NULL(CDR(expressions)));
				codegen_impl(CAR(expressions), code, tail_flag, 0, scope);
			}
			Array_Append(jump_forward_indices, (size_t[]) {SIZE(code)});
			NEW_INSTRUCTION(code, VM_JUMP_FORWARD);
		}
	}

	if (!has_else) {
		Array_Append(test_begin_indices, (size_t[]) {SIZE(code)});
		NEW_INSTRUCTION(code, VM_LOAD, NewRef(OBJ_NONE));
	}
	assert(SIZE(jump_if_false_indices) + 1 == SIZE(test_begin_indices));
	for (size_t i = 0; i < SIZE(jump_if_false_indices); ++i) {
		size_t src = Array_At(size_t, jump_if_false_indices, i);
		size_t dest = Array_At(size_t, test_begin_indices, i + 1);
		assert(src < dest);
		Code_SetOpArg(code, src, dest - src);
	}
	for (size_t i = 0, dest = SIZE(code); i < SIZE(jump_forward_indices); ++i) {
		size_t src = Array_At(size_t, jump_forward_indices, i);
		assert(src < dest);
		Code_SetOpArg(code, src, SIZE(code) - src);
	}
	DECREF(test_begin_indices);
	DECREF(jump_if_false_indices);
	DECREF(jump_forward_indices);
//	NEW_INSTRUCTION(code, VM_LOAD, AS_OBJECT(String_FromCStr("cond end")));
//	NEW_INSTRUCTION(code, VM_POP);
}

void codegen_impl(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope) {

	if (IS_LIST(o) && IS_NOT_NULL(o)) {
		if (EQV(global_symbols.quote, CAR(o))) {
			NEW_INSTRUCTION(code, VM_LOAD, NewRef(CAR(CDR(o))));
			return;
		} else if (EQV(global_symbols.if_expr, CAR(o))) {
			codegen_if(o, code, tail_flag, 0, scope);
			return;
		} else if (EQV(global_symbols.lambda, CAR(o))) {
			Object *formals = NULL;
			Object *body = NULL;

			assert(structural_match(o, "(lambda ? . ?)", &formals, &body) == 2);
			assert(Procedure_FormalsCheckValid(formals));
			assert(Procedure_FormalsCheckUnique(formals));

			NEW_INSTRUCTION(code, VM_LOAD, NewRef(formals));

			CodeObject *code1 = Code_New(NULL);
			codegen_lambda_body(body, code1, 0, 1, NULL);
			Code_Freeze(code1);

			NEW_INSTRUCTION(code, VM_LOAD, AS_OBJECT(code1));
			NEW_INSTRUCTION(code, VM_MAKE_FUNCTION);
			return;

		} else if (EQV(global_symbols.set, CAR(o))) {
			codegen_set(o, code, 0, 0, scope);
			return;
		} else if (EQV(global_symbols.define, CAR(o))) {
			codegen_define(o, code, 0, define_flag, scope);
			return;
		} else if (EQV(global_symbols.let, CAR(o))) {
			codegen_let(o, code, tail_flag, define_flag, scope);
			return;
		} else if (EQV(global_symbols.cond, CAR(o))) {
			codegen_cond(o, code, tail_flag, define_flag, scope);
			return;
		} else { // call
			size_t nargs = 0;
			codegen_impl(CAR(o), code, 0, 0, scope);
			for (Object *x = CDR(o); IS_NOT_NULL(x); x = CDR(x)) {
				codegen_impl(CAR(x), code, 0, 0, scope);
				nargs++;
			}
			NEW_INSTRUCTION(code, VM_LOAD, NewRef(EMPTY_LIST));
			for (size_t i = 0; i < nargs; ++i) NEW_INSTRUCTION(code, VM_CONS);
			NEW_INSTRUCTION(code, tail_flag ? VM_TAIL_CALL : VM_CALL);
			return;
		}
	} else {
		if (IS_TYPE(o, Boolean_Type) || IS_TYPE(o, Char_Type)
			|| IS_NONE(o) || IS_TYPE(o, Complex_Type)
			|| IS_TYPE(o, Float_Type) || IS_TYPE(o, Fraction_Type)
			|| IS_TYPE(o, Long_Type) || IS_TYPE(o, String_Type)
			|| IS_TYPE(o, Symbol_Type) || IS_TYPE(o, Vector_Type)) {

			NEW_INSTRUCTION(code, VM_LOAD, NewRef(o));
		} else {
			assert(0);
		}
		if (IS_TYPE(o, Symbol_Type)) {
			NEW_INSTRUCTION(code, VM_LOOKUP);
		}
		return;
	}
	assert(0);
}

CodeObject *codegen(Object *o, int tail_flag, int define_flag, ChainMap *scope) {
	CodeObject *code = Code_New(NULL);
	codegen_impl(o, code, tail_flag, define_flag, scope);
	Code_Freeze(code);
	return code;
}