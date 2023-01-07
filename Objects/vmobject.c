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
#include "../library/builtins.h"
#include "../Parser/parser.h"

TypeObject VirtualMachine_Type = {
		.tp_name = "VirtualMachine",
		.tp_basicsize=sizeof(VirtualMachineObject),
		.tp_itemsize=0,
		.tp_print=(print_proc) NULL,
		.tp_repr=(print_proc) NULL,
		.tp_dealloc =(dealloc_proc) VirtualMachine_Dealloc,
		.tp_search=(search_proc) VirtualMachine_Search
//		.tp_flags = TPFLAGS_HAVE_GC,
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
	Object *tmp = NULL;
	MOVE_SET(tmp, vm->operands, CONS(o, vm->operands));
	DECREF(o);
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

//// steals o.
//void VirtualMachine_Push(VirtualMachineObject *self, Object *o) {
//	push(self, o);
//}

//void VirtualMachine_Pop(VirtualMachineObject *self) {
//	pop(self);
//}

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
	} else {
		FrameObject *tmp2 = NULL;
		MOVE_SET(tmp2, self->frame, Frame_New(self->frame, func));
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
//		FrameObject *tmp2;
//		MOVE_SET(tmp2, self->frame, Frame_New(self->frame->fr_prev, CAST(ProcedureObject * , func)));
		FrameObject *tmp2 = Frame_New(self->frame->fr_prev, func);
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
	CLEAR(vm->operands);
	CLEAR(vm->globals);
	CLEAR(vm->frame);
}

void VirtualMachine_Search(VirtualMachineObject *vm, Object *target, ArrayObject *res) {
	APPEND_PARENT(target, vm, vm->operands);
	APPEND_PARENT(target, vm, vm->globals);
	APPEND_PARENT(target, vm, vm->frame);
	SEARCH(vm->operands, target, res);
	SEARCH(vm->globals, target, res);
	SEARCH(vm->frame, target, res);
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
			const VMInstruction *instr = Code_InstructionAt(vm->frame->fr_code, vm->frame->pc);
			switch (instr->opcode) {
				case VM_NOP:
					vm->frame->pc++;
					break;
				case VM_POP:
					pop(vm);
					vm->frame->pc++;
					break;
				case VM_SWAP: {
					Object *y = top(vm->operands);
					pop(vm);
					Object *x = top(vm->operands);
					pop(vm);
					push(vm, y);
					push(vm, x);
					vm->frame->pc++;
					break;
				}
				case VM_COPY: {
					Object *x = top(vm->operands);
					push(vm, x);
					vm->frame->pc++;
					break;
				}
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
					Object *x = top(vm->operands);
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
					DECREF(x);
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
				case VM_CALL: {
					Object *args = top(vm->operands);
					pop(vm);
					Object *func = top(vm->operands);
					pop(vm);
					assert(IS_TYPE(func, Procedure_Type));
					vm->frame->pc++;
					if (VirtualMachine_Call(vm, args, AS_PROCEDURE(func)) < 0)
						goto error;
#ifdef __DEBUG__
						if (CAST(ProcedureObject*, func)->c_function) {
							printf("call ");
							PRINT(func, stdout);
							printf(", arg=");
							PRINT(args, stdout);
							printf(", return value: ");
							PRINT(CAR(vm->operands), stdout);
							printf("\n");
						} else {
							printf("call\n");
							printf("----------frames---------\n");
							PRINT(vm->frame, stdout);
							printf("----------end frames-----\n");
						}
#endif
					DECREF(args);
					DECREF(func);
					break;
				}
				case VM_TAIL_CALL: {
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
					DECREF(x), DECREF(y);
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
			FrameObject *tmp = NULL;
			MOVE_SET(tmp, vm->frame, XNEW_REF(vm->frame->fr_prev));

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





