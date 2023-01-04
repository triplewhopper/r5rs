#ifndef R5RS_FRAME_OBJECT_H
#define R5RS_FRAME_OBJECT_H
struct frame_object {
	Object ob_base;
	struct frame_object *fr_prev;
	ChainMap *fr_env;
	CodeObject *fr_code;
	size_t pc;
};
extern TypeObject Frame_Type;

FrameObject *Frame_New(FrameObject *, ProcedureObject *);

FrameObject *Frame_Global(ChainMap *globals, CodeObject *code);

void Frame_Print(FrameObject *, FILE *);

void Frame_Dealloc(FrameObject *);

#endif //R5RS_FRAME_OBJECT_H
