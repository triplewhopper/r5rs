#ifndef R5RS_CODEGEN_H
#define R5RS_CODEGEN_H
#define NEW_INSTRUCTION(code, ...) Code_Append(code, (VMInstruction) {__VA_ARGS__})

void dfs(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope);

static void dfs_if(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope);

static void dfs_define(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope);

static void dfs_set(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope);

//static void dfs_lambda(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope);

static void dfs_lambda_body(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope);

void dfs_lambda_body(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope) {
	assert(IS_LIST(o));
	assert(IS_NOT_NULL(o));
	assert(define_flag);
	for (; IS_NOT_NULL(o); o = CDR(o)) {
		assert(IS_NOT_NULL(CAR(o)));
		if (IS_TYPE(CAR(o), Pair_Type) && !EQV(CAR(CAR(o)), global_symbols.define)) {
			define_flag = 0;
		}
		if (IS_NOT_NULL(CDR(o))) {
			dfs(CAR(o), code, 0, define_flag, scope);
			NEW_INSTRUCTION(code, VM_POP);
		} else {
			dfs(CAR(o), code, tail_flag, define_flag, scope);
		}
	}
}

void dfs_define(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope) {
	assert(define_flag != 0);
	Object *var = NULL, *formals = NULL, *e = NULL;
	if (ParseArgs(o, "(define (? . ?) . ?)", &var, &formals, &e) == 3) {
		assert(IS_TYPE(var, Symbol_Type));
		assert(Procedure_FormalsCheckValid(formals));
		assert(Procedure_FormalsCheckUnique(formals));

		NEW_INSTRUCTION(code, VM_LOAD, NewRef(var));
		NEW_INSTRUCTION(code, VM_LOAD, NewRef(formals));

		CodeObject *code1 = Code_New(var);
		dfs_lambda_body(e, code1, 1, 1, scope);
		Code_Freeze(code1);

		NEW_INSTRUCTION(code, VM_LOAD, AS_OBJECT(code1));
		NEW_INSTRUCTION(code, VM_MAKE_FUNCTION);
		NEW_INSTRUCTION(code, VM_DEFINE);
		NEW_INSTRUCTION(code, VM_LOAD, NewRef(OBJ_NONE));
		return;
	} else if (ParseArgs(o, "(define ? ?)", &var, &e) == 2) {
		assert(IS_TYPE(var, Symbol_Type));

		NEW_INSTRUCTION(code, VM_LOAD, NewRef(var));
		dfs(e, code, 1, 0, scope);
		NEW_INSTRUCTION(code, VM_DEFINE);
		NEW_INSTRUCTION(code, VM_LOAD, NewRef(OBJ_NONE));
		return;
	}
	assert(0);
}

void dfs_if(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope) {
	Object *test = NULL;
	Object *e1 = NULL, *e2 = NULL;
	if (ParseArgs(o, "(if ? ? ?)", &test, &e1, &e2) == 3) {
		dfs(test, code, 0, 0, scope);
		const size_t tmp = SIZE(code);
		NEW_INSTRUCTION(code, VM_JUMP_IF_FALSE);

		dfs(e1, code, tail_flag, 0, scope);
		const size_t tmp2 = SIZE(code);
		NEW_INSTRUCTION(code, VM_JUMP_FORWARD);

		dfs(e2, code, tail_flag, 0, scope);

		Code_SetOpArg(code, tmp, tmp2 + 1 - tmp);
		Code_SetOpArg(code, tmp2, SIZE(code) - tmp2);

	} else if (ParseArgs(o, "(if ? ?)", &test, &e1) == 2) {
		dfs(test, code, tail_flag, 0, scope);
		const size_t tmp = SIZE(code);
		NEW_INSTRUCTION(code, VM_JUMP_IF_FALSE);

		dfs(e1, code, tail_flag, 0, scope);
		const size_t tmp2 = SIZE(code);
		NEW_INSTRUCTION(code, VM_JUMP_FORWARD);

		NEW_INSTRUCTION(code, VM_LOAD, NewRef(OBJ_NONE));

		Code_SetOpArg(code, tmp, tmp2 + 1 - tmp);
		Code_SetOpArg(code, tmp2, SIZE(code) - tmp2);

	} else {
		assert(0);
	}
}

static void dfs_set(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope) {
	Object *var = NULL, *e = NULL;
	assert(ParseArgs(o, "(set! ? ?)", &var, &e) == 2);
	assert(IS_TYPE(var, Symbol_Type));
	NEW_INSTRUCTION(code, VM_LOAD, NewRef(var));
	dfs(e, code, 0, 0, scope);
	NEW_INSTRUCTION(code, VM_DEEP_STORE);
	NEW_INSTRUCTION(code, VM_LOAD, NewRef(OBJ_NONE));
}

static void dfs_let(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope) {
	Object *bindings = NULL, *name = NULL, *body = NULL, *b = NULL, *tmp;
	if (ParseArgs(o, "(let ? ? ?)", &name, &bindings, &body) == 3) {
		assert(IS_TYPE(name, Symbol_Type));
	} else {
		name = NULL;
		assert(ParseArgs(o, "(let ? ?)", &bindings, &body) == 2);
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
		assert(ParseArgs(CAR(b), "(? ?)", &var, &init) == 2);
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
	dfs(body, code1, tail_flag, 0, NULL);
	Code_Freeze(code1);
	NEW_INSTRUCTION(code, VM_LOAD, AS_OBJECT(code1));
	NEW_INSTRUCTION(code, VM_MAKE_FUNCTION);
	for (i = 0; i < n_bindings; ++i) {
		dfs(_inits[i], code, 0, 0, scope);
	}
	NEW_INSTRUCTION(code, VM_LOAD, NewRef(EMPTY_LIST));
	for (i = 0; i < n_bindings; ++i) {
		NEW_INSTRUCTION(code, VM_CONS);
	}
	NEW_INSTRUCTION(code, tail_flag ? VM_TAIL_CALL : VM_CALL);
	free(_formals);
	free(_inits);

}

static void dfs_cond(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope) {
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
				dfs(CAR(expressions), code, 0, 0, scope);
				NEW_INSTRUCTION(code, VM_POP);
			}
			assert(IS_NULL(CDR(expressions)));
			dfs(CAR(expressions), code, tail_flag, 0, scope);

		} else {
			Object *test = CAR(expressions);
			expressions = CDR(expressions);

			Array_Append(test_begin_indices, (size_t[]) {SIZE(code)});
			dfs(test, code, 0, 0, scope);
			if (len == 1) {
				NEW_INSTRUCTION(code, VM_COPY);
			}
			Array_Append(jump_if_false_indices, (size_t[]) {SIZE(code)});
			NEW_INSTRUCTION(code, VM_JUMP_IF_FALSE);
			if (len == 3 && EQV(global_symbols.cond_arrow, CAR(expressions))) {

				dfs(CAR(CDR(expressions)), code, 0, 0, scope);
				NEW_INSTRUCTION(code, VM_SWAP);
				NEW_INSTRUCTION(code, VM_LOAD, NewRef(EMPTY_LIST));
				NEW_INSTRUCTION(code, VM_CONS);
				NEW_INSTRUCTION(code, tail_flag ? VM_TAIL_CALL : VM_CALL);

			} else if (len >= 2) {

				for (; IS_NOT_NULL(CDR(expressions)); expressions = CDR(expressions)) {
					dfs(CAR(expressions), code, 0, 0, scope);
					NEW_INSTRUCTION(code, VM_POP);
				}
				assert(IS_NULL(CDR(expressions)));
				dfs(CAR(expressions), code, tail_flag, 0, scope);
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

void dfs(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope) {

	if (IS_LIST(o) && IS_NOT_NULL(o)) {
		if (EQV(global_symbols.quote, CAR(o))) {
			NEW_INSTRUCTION(code, VM_LOAD, NewRef(CAR(CDR(o))));
			return;
		} else if (EQV(global_symbols.if_expr, CAR(o))) {
			dfs_if(o, code, tail_flag, 0, scope);
			return;
		} else if (EQV(global_symbols.lambda, CAR(o))) {
			Object *formals = NULL;
			Object *body = NULL;

			assert(ParseArgs(o, "(lambda ? . ?)", &formals, &body) == 2);
			assert(Procedure_FormalsCheckValid(formals));
			assert(Procedure_FormalsCheckUnique(formals));

			NEW_INSTRUCTION(code, VM_LOAD, NewRef(formals));

			CodeObject *code1 = Code_New(NULL);
			dfs_lambda_body(body, code1, 0, 1, NULL);
			Code_Freeze(code1);

			NEW_INSTRUCTION(code, VM_LOAD, AS_OBJECT(code1));
			NEW_INSTRUCTION(code, VM_MAKE_FUNCTION);
			return;

		} else if (EQV(global_symbols.set, CAR(o))) {
			dfs_set(o, code, 0, 0, scope);
			return;
		} else if (EQV(global_symbols.define, CAR(o))) {
			dfs_define(o, code, 0, define_flag, scope);
			return;
		} else if (EQV(global_symbols.let, CAR(o))) {
			dfs_let(o, code, tail_flag, define_flag, scope);
			return;
		} else if (EQV(global_symbols.cond, CAR(o))) {
			dfs_cond(o, code, tail_flag, define_flag, scope);
			return;
		} else { // call
			size_t nargs = 0;
			dfs(CAR(o), code, 0, 0, scope);
			for (Object *x = CDR(o); IS_NOT_NULL(x); x = CDR(x)) {
				dfs(CAR(x), code, 0, 0, scope);
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

#endif //R5RS_CODEGEN_H
