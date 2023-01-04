#ifndef R5RS_CODE_OBJECT_H
#define R5RS_CODE_OBJECT_H

#include "object.h"

enum VMOpCode {
	VM_NOP,
	VM_POP,
	VM_DEFINE,
	VM_LOOKUP,
	VM_LOAD, // obj
	VM_DEEP_STORE,
	VM_CALL,
	VM_TAIL_CALL,
	VM_CONS,
	VM_MAKE_FUNCTION,
	VM_JUMP_FORWARD, // op_arg
	VM_JUMP_IF_FALSE, // op_arg
};

typedef struct vm_instruction {
	enum VMOpCode opcode;

	union {
		Object *obj;
		size_t op_arg;
	} operand;
} VMInstruction;

struct code_object {
	VarObject ob_base;
	size_t capacity;
	int frozen;
	size_t co_id;
//	size_t n_vars, n_consts;
	Object *co_name;
	VMInstruction *co_instructions;
};
extern TypeObject Code_Type;

CodeObject *Code_New(Object *co_name);

void Code_Append(CodeObject *self, VMInstruction instr);

void Code_Freeze(CodeObject *self);

void Code_Print(CodeObject *self, FILE *f);

void Code_Dealloc(CodeObject *self);

void Code_SetOpArg(CodeObject *self, size_t index, size_t op_arg);

void Code_SetOpObj(CodeObject *self, size_t index, Object *obj);

void Code_SetName(CodeObject *self, Object *name);

#endif //R5RS_CODE_OBJECT_H
