#ifndef R5RS_VM_OBJECT_H
#define R5RS_VM_OBJECT_H

#include "object.h"

struct virtual_machine_object {
	Object ob_base;
	Object *operands;
	ChainMap *globals;
	FrameObject *frame;
};
extern TypeObject VirtualMachine_Type;


VirtualMachineObject *VirtualMachine_New(ChainMap *);

void VirtualMachine_Dealloc(VirtualMachineObject *);
void VirtualMachine_Search(VirtualMachineObject *vm, Object *target, ArrayObject *res);
//void VirtualMachine_Push(VirtualMachineObject *, Object *);

//void VirtualMachine_Pop(VirtualMachineObject *);

int VirtualMachine_Exec(VirtualMachineObject *vm, CodeObject *code);

int VirtualMachine_EvalFrame(VirtualMachineObject *);

#endif //R5RS_VM_OBJECT_H
