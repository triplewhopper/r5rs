#include "../Include/typeobject.h"
#include "../Include/codeobject.h"

TypeObject Code_Type = {
		.tp_name="code",
		.tp_basicsize=sizeof(CodeObject),
		.tp_itemsize=sizeof(VMInstruction),
		.tp_dealloc=(dealloc_proc) Code_Dealloc,
		.tp_print=(print_proc) Code_Print,
		.tp_flags=TPFLAGS_HAVE_GC,
		.tp_traverse=(traverse_proc) Code_Traverse,
		.tp_search=(search_proc) Code_Search
};
static size_t code_id_count = 0;
CodeObject *codes_table[100];

#define VMInstructionAt(self, index) Array_At(VMInstruction, (self)->co_instructions, (index))

CodeObject *Code_New(Object *co_name) {
	CodeObject *res = CAST(CodeObject*, TypeGenericAlloc(&Code_Type, 0));
	assert(co_name == NULL || IS_TYPE(co_name, Symbol_Type));
	res->frozen = 0;
	res->co_id = ++code_id_count;
	codes_table[res->co_id] = res;
	res->co_name = XNewRef(co_name);
	res->co_instructions = Array_New(0, Code_Type.tp_itemsize);
	gc_track(AS_OBJECT(res));
	return res;
}

void Code_Append(CodeObject *self, VMInstruction instr) {
	assert(!self->frozen);
	SIZE(self)++;
	Array_Append(self->co_instructions, &instr);
	if (instr.opcode == VM_LOAD) assert(instr.operand.obj->ob_type);
	else
		assert(!instr.operand.op_arg);
}

void Code_Freeze(CodeObject *self) {
	if (!self->frozen) self->frozen = 1;
}

void Code_Print(CodeObject *self, FILE *f) {
#define MAKE_CASE(x) case x: fprintf(f, #x "\n"); break
	ArrayObject *linked_code = Array_New(0, sizeof(CodeObject *));
	fprintf(f, "code object ");
	if (self->co_name) {
		PRINT(self->co_name, f);
		fprintf(f, " ");
	}
	fprintf(f, "(id=%zu): \n", self->co_id);
	for (size_t i = 0; i < SIZE(self); ++i) {
		fprintf(f, "%3zu ", i);
		const VMInstruction *const instr = &VMInstructionAt(self, i);
		switch (instr->opcode) {
			MAKE_CASE(VM_NOP);
			MAKE_CASE(VM_POP);
			MAKE_CASE(VM_SWAP);
			MAKE_CASE(VM_COPY);
			MAKE_CASE(VM_DEFINE);
			MAKE_CASE(VM_LOOKUP);
			case VM_LOAD: {
				fprintf(f, "VM_LOAD ");
				Object *o = instr->operand.obj;
				fprintf(f, "%s object ", TYPE(o)->tp_name);
				if (!IS_TYPE(o, Code_Type)) {
					PRINT(o, f);
				} else {
					CodeObject *co = CAST(CodeObject*, o); // borrow
					Array_Append(linked_code, &co);
//					linked_code[n_linked_code++] = co;
					if (co->co_name) {
						PRINT(co->co_name, f);
						fprintf(f, " (#%zu)", co->co_id);
					} else {
						fprintf(f, "#%zu", co->co_id);
					}
				}
				fputc('\n', f);
				break;
			}
			MAKE_CASE(VM_DEEP_STORE);
			MAKE_CASE(VM_CALL);
			MAKE_CASE(VM_TAIL_CALL);
			MAKE_CASE(VM_CONS);
			MAKE_CASE(VM_MAKE_FUNCTION);
			case VM_JUMP_FORWARD:
				fprintf(f, "VM_JUMP_FORWARD %zu\n", instr->operand.op_arg);
				break;
			case VM_JUMP_IF_FALSE:
				fprintf(f, "VM_JUMP_IF_FALSE %zu\n", instr->operand.op_arg);
				break;

		}
	}
	fputc('\n', f);
	for (size_t i = 0; i < SIZE(linked_code); ++i) {
		CodeObject *co = Array_At(CodeObject*, linked_code, i);
		if (co->co_id > self->co_id)
			Code_Print(co, f);
	}
	DECREF(linked_code);
#undef MAKE_CASE
}

void Code_GC_Clear(CodeObject *self) {
	codes_table[self->co_id] = 0;
	assert(SIZE(self) == SIZE(self->co_instructions));
	for (size_t i = 0; i < SIZE(self); ++i) {
		VMInstruction *const instr = &VMInstructionAt(self, i);
		if (instr->opcode == VM_LOAD) {
			CLEAR(instr->operand.obj);
		}
	}
	CLEAR(self->co_instructions);
	XDECREF(self->co_name);

}

void Code_Dealloc(CodeObject *self) {
	gc_untrack(AS_OBJECT(self));
	Code_GC_Clear(self);
}

extern Object *live_objects[100000];

void Code_Traverse(CodeObject *self, visit_proc visit, void *arg) {
	assert(SIZE(self) == SIZE(self->co_instructions));
	assert(REFCNT(self) > 0);
	if(self->co_name) VISIT(self->co_name, arg);
	for (size_t i = 0; i < SIZE(self); ++i) {
		VMInstruction *const instr = &VMInstructionAt(self, i);
		if (instr->opcode == VM_LOAD) {
			VISIT(instr->operand.obj, arg);
		}
	}
	VISIT(self->co_instructions, arg);
}

void Code_Search(CodeObject *self, Object *target, ArrayObject *res) {
	APPEND_PARENT(target, self, self->co_name);
	for (size_t i = 0; i < SIZE(self); ++i) {
		VMInstruction *const instr = &VMInstructionAt(self, i);
		if (instr->opcode == VM_LOAD) {
			APPEND_PARENT(target, self, instr->operand.obj);
		}
	}

	SEARCH(self->co_name, target, res);
	for (size_t i = 0; i < SIZE(self); ++i) {
		VMInstruction *const instr = &VMInstructionAt(self, i);
		if (instr->opcode == VM_LOAD) {
			SEARCH(instr->operand.obj, target, res);
		}
	}

}

void Code_SetOpArg(CodeObject *self, size_t index, size_t op_arg) {
	assert(index < SIZE(self));
	VMInstruction *const i = &VMInstructionAt(self, index);
	assert(i->opcode == VM_JUMP_FORWARD || i->opcode == VM_JUMP_IF_FALSE);
	assert(i->operand.op_arg == 0);
	i->operand.op_arg = op_arg;
}

void Code_SetOpObj(CodeObject *self, size_t index, Object *obj) {
	assert(index < SIZE(self));
	VMInstruction *const i = &VMInstructionAt(self, index);
	assert(i->opcode == VM_LOAD);
	i->operand.obj = NewRef(obj);
}

void Code_SetName(CodeObject *self, Object *name) {
	assert(IS_TYPE(name, Symbol_Type));
	assert(self->co_name == NULL);
	self->co_name = NewRef(name);
}

const VMInstruction *Code_InstructionAt(CodeObject *self, size_t index) {
	return &VMInstructionAt(self, index);
}

#undef VMInstructionAt