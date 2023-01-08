#ifndef R5RS_CODEGEN_H
#define R5RS_CODEGEN_H

CodeObject *codegen(Object *o, int tail_flag, int define_flag, ChainMap *scope);

#endif //R5RS_CODEGEN_H
