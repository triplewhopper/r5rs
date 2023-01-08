#ifndef R5RS_CODE_OBJECT_H
#define R5RS_CODE_OBJECT_H

#include "object.h"
#include "arrayobject.h"

enum VMOpCode {
	VM_NOP,
	VM_POP,
	VM_SWAP,
	VM_COPY,
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
	int frozen;
	size_t co_id;
	Object *co_name;
	ArrayObject *co_instructions;
};

CodeObject *Code_New(Object *co_name);

void Code_Append(CodeObject *self, VMInstruction instr);

#define NEW_INSTRUCTION(code, ...) Code_Append(code, (VMInstruction) {__VA_ARGS__})

void Code_Freeze(CodeObject *self);

void Code_Print(CodeObject *self, FILE *f);

void Code_Dealloc(CodeObject *self);

void Code_Traverse(CodeObject *self, visit_proc visit, void *arg);

void Code_Search(CodeObject *self, Object *target, ArrayObject *res);

void Code_SetOpArg(CodeObject *self, size_t index, size_t op_arg);

void Code_SetOpObj(CodeObject *self, size_t index, Object *obj);

void Code_SetName(CodeObject *self, Object *name);

const VMInstruction *Code_InstructionAt(CodeObject *self, size_t index);

#endif //R5RS_CODE_OBJECT_H
