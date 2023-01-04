#include "../Include/typeobject.h"
#include "../Include/codeobject.h"

TypeObject Code_Type = {
		.tp_name="code",
		.tp_basicsize=sizeof(CodeObject),
		.tp_itemsize=sizeof(VMInstruction),
		.tp_dealloc=(dealloc_proc) Code_Dealloc,
		.tp_print=(print_proc) Code_Print,
		.tp_flags=TPFLAGS_HAVE_GC,
};
static size_t code_id_count = 0;
CodeObject *codes_table[100];

CodeObject *Code_New(Object *co_name) {
	CodeObject *res = CAST(CodeObject*, TypeGenericAlloc(&Code_Type, 0));
	assert(co_name == NULL || IS_TYPE(co_name, Symbol_Type));
	res->capacity = 1;
	res->frozen = 0;
	res->co_id = ++code_id_count;
	codes_table[res->co_id] = res;
	res->co_name = XNewRef(co_name);
	res->co_instructions = calloc(1, Code_Type.tp_itemsize);
	assert(res->co_instructions);

	return res;
}

static void Code_Reserve(CodeObject *self, size_t capacity) {
	assert(!self->frozen);
	if (self->capacity < capacity) {
		self->co_instructions = reallocf(self->co_instructions, capacity * TYPE(self)->tp_itemsize);
		assert(self->co_instructions);
		memset(self->co_instructions + self->capacity, 0,
			   (capacity - self->capacity) * TYPE(self)->tp_itemsize);
	}
}

void Code_Append(CodeObject *self, VMInstruction instr) {
	assert(!self->frozen);
	while (SIZE(self) + 1 >= self->capacity) {
		Code_Reserve(self, self->capacity * 2);
		self->capacity *= 2;
	}
	assert(self->co_instructions[SIZE(self)].opcode == VM_NOP);
	self->co_instructions[SIZE(self)++] = instr;
	if (instr.opcode == VM_LOAD) assert(instr.operand.obj->ob_type);
	else
		assert(!instr.operand.op_arg);
}

void Code_Freeze(CodeObject *self) {
	if (!self->frozen) self->frozen = 1;
}

void Code_Print(CodeObject *self, FILE *f) {
#define MAKE_CASE(x) case x: fprintf(f, #x "\n"); break
	CodeObject *linked_code[100];
	size_t n_linked_code = 0;
	fprintf(f, "code object ");
	if (self->co_name) {
		PRINT(self->co_name, f);
		fprintf(f, " ");
	}
	fprintf(f, "(id=%zu): \n", self->co_id);
	for (size_t i = 0; i < SIZE(self); ++i) {
		fprintf(f, "%3zu ", i);
		switch (self->co_instructions[i].opcode) {
			MAKE_CASE(VM_NOP);
			MAKE_CASE(VM_POP);
			MAKE_CASE(VM_DEFINE);
			MAKE_CASE(VM_LOOKUP);
			case VM_LOAD: {
				fprintf(f, "VM_LOAD ");
				Object *o = self->co_instructions[i].operand.obj;
				fprintf(f, "%s object ", TYPE(o)->tp_name);
				if (!IS_TYPE(o, Code_Type)) {
					PRINT(o, f);
				} else {
					CodeObject *co = CAST(CodeObject*, o);
					linked_code[n_linked_code++] = co;
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
				fprintf(f, "VM_JUMP_FORWARD %zu\n", self->co_instructions[i].operand.op_arg);
				break;
			case VM_JUMP_IF_FALSE:
				fprintf(f, "VM_JUMP_IF_FALSE %zu\n", self->co_instructions[i].operand.op_arg);
				break;

		}
	}
	fputc('\n', f);
	for (size_t i = 0; i < n_linked_code; ++i) {
		if (linked_code[i]->co_id > self->co_id)
			Code_Print(linked_code[i], f);
	}
#undef MAKE_CASE
}

void Code_Dealloc(CodeObject *self) {
	gc_untrack(AS_OBJECT(self));
	codes_table[self->co_id] = 0;
	for (size_t i = 0; i < SIZE(self); ++i) {
		if (self->co_instructions[i].opcode == VM_LOAD) {
			DECREF(self->co_instructions[i].operand.obj);
		}
	}
	free(self->co_instructions);
	XDECREF(self->co_name);
}


void Code_SetOpArg(CodeObject *self, size_t index, size_t op_arg) {
	assert(index < SIZE(self));
	VMInstruction *const i = &self->co_instructions[index];
	assert(i->opcode == VM_JUMP_FORWARD || i->opcode == VM_JUMP_IF_FALSE);
	assert(i->operand.op_arg == 0);
	i->operand.op_arg = op_arg;
}

void Code_SetOpObj(CodeObject *self, size_t index, Object *obj) {
	assert(index < SIZE(self));
	VMInstruction *const i = &self->co_instructions[index];
	assert(i->opcode == VM_LOAD);
	i->operand.obj = NewRef(obj);
}

void Code_SetName(CodeObject *self, Object *name) {
	assert(IS_TYPE(name, Symbol_Type));
	assert(self->co_name == NULL);
	self->co_name = NewRef(name);
}