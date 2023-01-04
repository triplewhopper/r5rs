#include <stdlib.h>
#include "../Include/typeobject.h"
#include "../Include/pairobject.h"
#include "../Include/chainmapobject.h"
#include "../Include/symbolobject.h"
#include "../Include/stringobject.h"
#include "../Include/noneobject.h"
#include "../Include/procedureobject.h"
#include "../Include/frameobject.h"
#include "../Include/numberobject.h"
#include "../Include/vmobject.h"
#include "../Include/codeobject.h"
#include "../library/runtime.h"
#include "../Parser/parser.h"

TypeObject VirtualMachine_Type = {
		.tp_name = "VirtualMachine",
		.tp_basicsize=sizeof(VirtualMachineObject),
		.tp_itemsize=0,
		.tp_print=(print_proc) NULL,
		.tp_repr=(print_proc) NULL,
		.tp_dealloc =(dealloc_proc) VirtualMachine_Dealloc
};

VirtualMachineObject *VirtualMachine_New(ChainMap *globals) {
	VirtualMachineObject *vm = CAST(VirtualMachineObject *, TypeGenericAlloc(&VirtualMachine_Type, 0));
	vm->operands = NewRef(EMPTY_LIST);
	vm->globals = NEW_REF(globals);
	vm->frame = NULL;
	return vm;

}

// steals o
static void push(VirtualMachineObject *vm, Object *o) {
	assert(o);
	vm->operands = CONS(o, vm->operands);
}

static void pop(VirtualMachineObject *vm) {

	assert(IS_OWNED(vm->operands));
	Object *cdr = NULL;
	cdr = CDR(vm->operands);
	DECREF(CAR(vm->operands));
	AS_PAIR(vm->operands)->cdr = NewRef(EMPTY_LIST);
	AS_PAIR(vm->operands)->car = NewRef(EMPTY_LIST);
	DECREF(vm->operands);
	vm->operands = cdr;
//	MOVE_SET(tmp, AS_PAIR(vm->operands)->car, NULL);
//	MOVE_SET(tmp, cdr,)
//	MOVE_SET(tmp, vm->operands, CDR(vm->operands));
//	MOVE_SET(tmp, vm->operands, NewRef(CDR(vm->operands)));
}

static Object *top(Object *stack) {
	return NewRef(CAR(stack));
}

// steals o.
void VirtualMachine_Push(VirtualMachineObject *self, Object *o) {
	push(self, o);
}

void VirtualMachine_Pop(VirtualMachineObject *self) {
	pop(self);
}

static Object *call_c_func(CFunction *f, Object *args) {
	size_t argc = Pair_ListLength(args), i = 0;
	Object **argv = calloc(argc + 1, sizeof(Object *));
	assert(argv);
	for (Object *t = args; IS_NOT_NULL(t); t = CDR(t)) {
		argv[i++] = CAR(t);
	}
	Object *res = f(argc, argv);
	free(argv);
	return res;
}

int VirtualMachine_Call(VirtualMachineObject *self, Object *args, ProcedureObject *func) {
	assert(IS_LIST(args));
	CFunction *c_func = func->c_function;
	if (c_func != NULL) {
		Object *res = call_c_func(c_func, args);
		if (res == NULL) return -1;
		push(self, res);
//		printf("return value ");
//		PRINT(res, stdout);
//		printf("\n");
//		DECREF(res);
	} else {
		FrameObject *tmp2 = NULL;
		MOVE_SET(tmp2, self->frame, Frame_New(self->frame, func));
//		FrameObject *tmp2 = Frame_New(self->frame, CAST(ProcedureObject *, func));
//		DECREF(self->frame);
//		self->frame = tmp2;
		if (IS_LIST(func->formals)) {
			assert(Pair_ListLength(args) == Pair_ListLength(func->formals));
			Object *t1, *t2;
			for (t1 = func->formals, t2 = args; IS_NOT_NULL(t1); t1 = CDR(t1), t2 = CDR(t2)) {
				assert(IS_TYPE(CAR(t1), Symbol_Type));
				ChainMap_SetItem(self->frame->fr_env, AS_SYMBOL(CAR(t1)), CAR(t2));
			}
		} else if (IS_TYPE(func->formals, Pair_Type)) {
			Object *t1, *t2;
			for (t1 = func->formals, t2 = args; IS_TYPE(t1, Pair_Type); t1 = CDR(t1), t2 = CDR(t2)) {
				assert(IS_TYPE(CAR(t1), Symbol_Type));
				assert(IS_NOT_NULL(t2));
				ChainMap_SetItem(self->frame->fr_env, AS_SYMBOL(CAR(t1)), CAR(t2));
			}
			assert(IS_TYPE(t1, Symbol_Type));
			ChainMap_SetItem(self->frame->fr_env, AS_SYMBOL(t1), t2);
		} else {
			assert(IS_TYPE(func->formals, Symbol_Type));
			ChainMap_SetItem(self->frame->fr_env, AS_SYMBOL(func->formals), args);
		}
	}
	return 0;
}

int VirtualMachine_TailCall(VirtualMachineObject *self, Object *args, ProcedureObject *func) {
	assert(IS_LIST(args));
	CFunction *c_func = AS_PROCEDURE(func)->c_function;
	if (c_func != NULL) {
		Object *res = call_c_func(c_func, args);
		if (res == NULL) return -1;
		push(self, res);
//		printf("return value ");
//		PRINT(res, stdout);
//		printf("\n");
//		DECREF(res);
	} else {
//		FrameObject *tmp2;
//		MOVE_SET(tmp2, self->frame, Frame_New(self->frame->fr_prev, CAST(ProcedureObject * , func)));
		FrameObject *tmp2 = Frame_New(self->frame->fr_prev, AS_PROCEDURE(func));
		DECREF(self->frame);
		self->frame = tmp2;
		if (IS_LIST(func->formals)) {
			assert(Pair_ListLength(args) == Pair_ListLength(func->formals));
			Object *t1, *t2;
			for (t1 = func->formals, t2 = args; IS_NOT_NULL(t1); t1 = CDR(t1), t2 = CDR(t2)) {
				assert(IS_TYPE(CAR(t1), Symbol_Type));
				ChainMap_SetItem(self->frame->fr_env, AS_SYMBOL(CAR(t1)), CAR(t2));
			}
		} else if (IS_TYPE(func->formals, Pair_Type)) {
			Object *t1, *t2;
			for (t1 = func->formals, t2 = args; IS_TYPE(t1, Pair_Type); t1 = CDR(t1), t2 = CDR(t2)) {
				assert(IS_TYPE(CAR(t1), Symbol_Type));
				assert(IS_NOT_NULL(t2));
				ChainMap_SetItem(self->frame->fr_env, AS_SYMBOL(CAR(t1)), CAR(t2));
			}
			assert(IS_TYPE(t1, Symbol_Type));
			ChainMap_SetItem(self->frame->fr_env, AS_SYMBOL(t1), t2);
		}
	}
	return 0;
}

void VirtualMachine_Dealloc(VirtualMachineObject *vm) {
	DECREF(vm->operands);
	DECREF(vm->globals);
	XDECREF(vm->frame);
}

int VirtualMachine_Exec(VirtualMachineObject *vm, CodeObject *code) {
	assert(vm->frame == NULL);
//	INCREF(code);
	vm->frame = Frame_Global(vm->globals, code);
	return VirtualMachine_EvalFrame(vm);
}

int VirtualMachine_EvalFrame(VirtualMachineObject *vm) {
	while (vm->frame) {
#ifdef __DEBUG__
		printf("----------frames---------\n");
		PRINT(vm->frame, stdout);
		printf("----------end frames-----\n");
#endif
		while (vm->frame->pc < SIZE(vm->frame->fr_code)) {
//			printf("pc=%zu/%d\n", vm->frame->pc, SIZE(vm->frame->fr_code));
			VMInstruction *instr = &vm->frame->fr_code->co_instructions[vm->frame->pc];
			switch (instr->opcode) {
				case VM_NOP:
					vm->frame->pc++;
					break;
				case VM_POP:
					pop(vm);
					vm->frame->pc++;
					break;
				case VM_DEFINE: {
					Object *y = top(vm->operands);
					pop(vm);
					Object *x = top(vm->operands);
					pop(vm);
					assert(IS_TYPE(x, Symbol_Type));
					ChainMap_SetItem(vm->frame->fr_env, AS_SYMBOL(x), y);
					DECREF(x), DECREF(y);
					vm->frame->pc++;
					break;
				}
				case VM_LOOKUP: {
					Object *x = CAR(vm->operands);
					pop(vm);
					assert(IS_TYPE(x, Symbol_Type));
					Object *y = ChainMap_GetItem(vm->frame->fr_env, AS_SYMBOL(x));
					if (y == NULL) {
						fprintf(stderr, "unbound variable: ");
						PRINT(AS_SYMBOL(x)->name, stderr);
						fprintf(stderr, "\n");
						DECREF(x);
						goto error;
					}
					push(vm, y);
//					DECREF(x);
//					DECREF(y);
					vm->frame->pc++;
					break;
				}
				case VM_LOAD: {
					push(vm, NewRef(instr->operand.obj));
					vm->frame->pc++;
					break;
				}
				case VM_DEEP_STORE: {
					Object *y = top(vm->operands);
					pop(vm);
					Object *x = top(vm->operands);
					pop(vm);
					assert(IS_TYPE(x, Symbol_Type));
					assert(ChainMap_Contains(vm->frame->fr_env, AS_SYMBOL(x)));
					ChainMap_DeepSetItem(vm->frame->fr_env, AS_SYMBOL(x), y);
					DECREF(x), DECREF(y);
					vm->frame->pc++;
					break;
				}
				case VM_CALL:
				case VM_TAIL_CALL: {
					Object *args = top(vm->operands);
					pop(vm);
					Object *func = top(vm->operands);
					pop(vm);
					assert(IS_TYPE(func, Procedure_Type));
					vm->frame->pc++;
					if (VirtualMachine_Call(vm, args, AS_PROCEDURE(func)) < 0)
						goto error;
					DECREF(args);
					DECREF(func);
#ifdef __DEBUG__
					if (CAST(ProcedureObject*, func)->c_function) {
						printf("call ");
						PRINT(func, stdout);
						printf("\n");
					} else {
						printf("call\n");
						printf("----------frames---------\n");
						PRINT(vm->frame, stdout);
						printf("----------end frames-----\n");
					}
#endif
					break;
				}
					{
						Object *args = top(vm->operands);
						pop(vm);
						Object *func = top(vm->operands);
						pop(vm);
						assert(IS_TYPE(func, Procedure_Type));
						vm->frame->pc++;
						if (vm->frame->fr_prev == NULL) {
							if (VirtualMachine_Call(vm, args, AS_PROCEDURE(func)) < 0)
								goto error;
#ifdef __DEBUG__
							printf("initial tail call\n");
#endif
						} else {
							if (VirtualMachine_TailCall(vm, args, AS_PROCEDURE(func)) < 0)
								goto error;
#ifdef __DEBUG__
							printf("tail call\n");
#endif
						}
#ifdef __DEBUG__
						printf("----------frames---------\n");
						PRINT(vm->frame, stdout);
						printf("----------end frames-----\n");
#endif
						DECREF(args);
						DECREF(func);
						break;
					}
				case VM_CONS: {
					Object *y = top(vm->operands);
					pop(vm);
					Object *x = top(vm->operands);
					pop(vm);
					push(vm, CONS(x, y));
					vm->frame->pc++;
					break;
				}
				case VM_MAKE_FUNCTION: {
					Object *body = top(vm->operands);
					pop(vm);
					Object *formals = top(vm->operands);
					pop(vm);
					assert(IS_TYPE(body, Code_Type));
					ProcedureObject *func = Procedure_New(formals, CAST(CodeObject *, body), vm->frame->fr_env);
					push(vm, AS_OBJECT(func));
					DECREF(body), DECREF(formals);
					vm->frame->pc++;
					break;
				}
				case VM_JUMP_FORWARD: {
					vm->frame->pc += instr->operand.op_arg;
					break;
				}
				case VM_JUMP_IF_FALSE: {
					assert(IS_NOT_NULL(vm->operands));
					Object *x = top(vm->operands);
					pop(vm);
					if (IS_FALSE(x)) {
						vm->frame->pc += instr->operand.op_arg;
					} else vm->frame->pc++;
					DECREF(x);
					break;
				}
			}
		}

		{
			assert(IS_OWNED(vm->frame));
			FrameObject *tmp = vm->frame->fr_prev;
			vm->frame->fr_prev = NULL;
			DECREF(vm->frame);
			vm->frame = tmp;

//			printf("return\n");
		}
	}
	return 0;
	error:
	fprintf(stderr, "error occurred.\n");
	fprintf(stderr, "traceback (most recent call first):\n");
	PRINT(vm->frame, stderr);
	DECREF(vm->frame);
	vm->frame = NULL;
	while (IS_NOT_NULL(vm->operands)) pop(vm);
	return -1;
}
//Object *translate(Object *o) {
//	if (IS_LIST(o)) {
//		if (CAR(o) == global_symbols.quote) {
//			assert(IS_NOT_NULL(CDR(o)));
//		} else if (CAR(o) == global_symbols.if_expr) {
//			Object *test = NULL;
//			Object *e1 = NULL, *e2 = NULL;
//			if (ParseArgs(o, "(if ? ? ?)", &test, &e1, &e2) == 3) {
//			} else if (ParseArgs(o, "(if ? ?)", &test, &e1) == 2) {
//			} else {
//				assert(0);
//			}
//
//		} else if (CAR(o) == global_symbols.lambda) {
//			Object *params = NULL;
//			Object *body = NULL;
//			assert(ParseArgs(o, "(lambda ? . ?)", &params, &body) == 2);
//			assert(IS_TYPE(params, Symbol_Type) || IS_LIST(params) || IS_TYPE(params, Pair_Type));
//			Object *translated_body = translate(body);
//			if (translated_body == body) {
//				DECREF(translated_body);
//				INCREF(o);
//				return o;
//			} else {
//				Object *tmp = CONS(params, translated_body);
//				Object *res = CONS(global_symbols.lambda, tmp);
//				DECREF(tmp);
//				return res;
//			}
//		} else if (CAR(o) == global_symbols.set) {
//			Object *var = NULL, *e = NULL;
//			assert(ParseArgs(o, "(set! ? ?)", &var, &e) == 2);
//			assert(IS_TYPE(var, Symbol_Type));
//			Object *translated_expr = translate(e);
//			if (translated_expr == e) {
//				DECREF(translated_expr);
//				INCREF(o);
//				return o;
//			} else {
//				Object *tmp = CONS(var, translated_expr);
//				Object *res = CONS(global_symbols.set, tmp);
//				DECREF(tmp);
//				return res;
//			}
//		} else if (CAR(o) == global_symbols.define) {
//			Object *var = NULL, *formals = NULL, *e = NULL;
//			if (ParseArgs(o, "(define (? . ?) ?)", &var, &formals, &e) == 3) {
//				assert(IS_TYPE(var, Symbol_Type));
//				assert(IS_LIST(formals) || IS_TYPE(formals, Pair_Type) || IS_TYPE(formals, Symbol_Type));
//				Object *translated_expr = translate(e);
//				Object *res = CONS(formals, translated_expr);
//				Object *tmp = NULL;
//				MOVE_SET(tmp, res, CONS(global_symbols.lambda, res));
//				MOVE_SET(tmp, res, CONS(var, res));
//				MOVE_SET(tmp, res, CONS(global_symbols.define, res));
//				DECREF(translated_expr);
//				return res;
//			} else if (ParseArgs(o, "(define ? ?)", &var, &e) == 2) {
//				assert(IS_TYPE(var, Symbol_Type));
//				Object *translated_expr = translate(e);
//				if (translated_expr == e) {
//					DECREF(translated_expr);
//					INCREF(o);
//					return o;
//				} else {
//					Object *tmp = CONS(var, translated_expr);
//					Object *res = CONS(global_symbols.define, tmp);
//					DECREF(tmp);
//					return res;
//				}
//			} else {
//				assert(0);
//			}
//		} else {
//			assert(0);
//		}
//	} else {
//		INCREF(o);
//		return o;
//	}
//}

#define NEW_INSTRUCTION(code, ...) Code_Append(code, (VMInstruction) {__VA_ARGS__})

void dfs(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope);

static void dfs_if(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope);

static void dfs_define(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope);

static void dfs_set(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope);

//static void dfs_lambda(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope);

static void dfs_lambda_body(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope);

void dfs_lambda_body(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope) {
	assert(code->co_id != 2 || SIZE(code) <= 9 || code->co_instructions[9].opcode == VM_JUMP_IF_FALSE);
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
	assert(code->co_id != 2 || SIZE(code) <= 9 || code->co_instructions[9].opcode == VM_JUMP_IF_FALSE);
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
//	assert(code->co_id != 2 || SIZE(code) <= 9 || code->co_instructions[9].opcode == VM_JUMP_IF_FALSE);
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
		formals = CONS(NewRef(_formals[n_bindings - i]), formals);
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

void dfs(Object *o, CodeObject *code, int tail_flag, int define_flag, ChainMap *scope) {
//	assert(code->co_id != 2 || SIZE(code) <= 9 || code->co_instructions[9].opcode == VM_JUMP_IF_FALSE);

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




