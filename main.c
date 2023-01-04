#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include "Lexer/lexer.h"
#include "Parser/parser.h"
#include "Include/typeobject.h"
#include "Include/pairobject.h"
#include "Include/symbolobject.h"
#include "Include/numberobject.h"
#include "Include/chainmapobject.h"
#include "Include/codeobject.h"
#include "Include/procedureobject.h"
#include "Include/frameobject.h"
#include "Include/vmobject.h"
#include "library/runtime.h"

void TypeObjectsInit() {
	Type_SetBaseClass(&Boolean_Type, &BaseObject_Type);
	Type_SetBaseClass(&Char_Type, &BaseObject_Type);
	Type_SetBaseClass(&Complex_Type, &BaseObject_Type);
	Type_SetBaseClass(&Float_Type, &BaseObject_Type);
	Type_SetBaseClass(&Fraction_Type, &BaseObject_Type);
	Type_SetBaseClass(&Long_Type, &BaseObject_Type);
	Type_SetBaseClass(&None_Type, &BaseObject_Type);
	Type_SetBaseClass(&Pair_Type, &BaseObject_Type);
	Type_SetBaseClass(&String_Type, &BaseObject_Type);
	Type_SetBaseClass(&Symbol_Type, &BaseObject_Type);
	Type_SetBaseClass(&Vector_Type, &BaseObject_Type);
	Type_SetBaseClass(&Token_Type, &BaseObject_Type);
	Type_SetBaseClass(&Dict_Type, &BaseObject_Type);
	Type_SetBaseClass(&DictNode_Type, &BaseObject_Type);
	Type_SetBaseClass(&ChainMap_Type, &BaseObject_Type);
	Type_SetBaseClass(&Procedure_Type, &BaseObject_Type);

	Type_SetBaseClass(&Code_Type, &BaseObject_Type);
	Type_SetBaseClass(&Frame_Type, &BaseObject_Type);
	Type_SetBaseClass(&VirtualMachine_Type, &BaseObject_Type);
}

const char *readline() {
	char *buf = NULL;
	size_t size = 0;
	FILE *f = open_memstream(&buf, &size);
	assert(f);
	int c;
	while ((c = getchar()) != '\n') fputc(c, f);
	fclose(f);
	return buf;
}

void bytecode_test() {
	LexerObject *l = Lexer_FromFile("../testcode/s3.scm");
	FILE *fout = fopen("../testcode/s3.bytecode.txt", "w");
	ChainMap *builtins = LoadBuiltinFuncs();
	ChainMap *globals = ChainMap_NewChild(builtins, NULL);
	VirtualMachineObject *vm = VirtualMachine_New(globals);
	Object *d = ParseDatum(l);
	int count = 0;
//	int ffflag = 0;
	while (d) {
#ifdef __DEBUG__
		printf("evaluating ");
		PRINT(d, stdout);
		printf("\n");
#endif
		CodeObject *code = Code_New(NULL);
		dfs(d, code, 0, 1, NULL);
		PRINT(code, fout);
//		if(ffflag){
//			DECREF(code);
//			1+2;
//			ffflag = 0;
//		}
		if (VirtualMachine_Exec(vm, code) < 0) {
			DECREF(vm);
			DECREF(code);
			break;
		} else {
			DECREF(code);

			assert(IS_NOT_NULL(vm->operands));
			assert(Pair_ListLength(vm->operands) == 1);
			printf("$%d=", ++count);
			PRINT(CAR(vm->operands), stdout);
			printf("\n");
			VirtualMachine_Pop(vm);
#ifdef __DEBUG__
			printf("\n");
#endif
		}
		DECREF(d);
		d = ParseDatum(l);
	}
//	Code_Freeze(code);
	fclose(fout);
	Lexer_Finalize(l);
	DECREF(vm);
	DECREF(globals);
	DECREF(builtins);

}

//void repl() {
//	ChainMap *builtins = LoadBuiltinFuncs();
//	ChainMap *globals = ChainMap_NewChild(builtins, NULL);
//	VirtualMachineObject *vm = VirtualMachine_New(globals);
//	while (1) {
//		const char *buf = readline();
//		LexerObject *l = Lexer_FromCStr(buf);
//#ifdef __DEBUG__
//		FILE *fout = fopen("../testcode/logicals.bytecode.txt", "w");
//#endif
////		INCREF(EMPTY_LIST);
//		Object *d = ParseDatum(l);
//		int count = 0;
//		while (d) {
//#ifdef __DEBUG__
//			printf("evaluating ");
//			PRINT(d, stdout);
//			printf("\n");
//#endif
//			CodeObject *code = Code_New(NULL);
//			dfs(d, code, 0, 1, NULL);
//#ifdef __DEBUG__
//			PRINT(code, fout);
//#endif
//			if (VirtualMachine_Exec(vm, code) < 0) {
//				DECREF(vm);
//				break;
//			} else {
//				assert(IS_NOT_NULL(vm->operands));
//				assert(Pair_ListLength(vm->operands) == 1);
//				printf("$%d=", ++count);
//				char tmp[30];
//				sprintf(tmp, "$%d", count);
//				PRINT(CAR(vm->operands), stdout);
//				printf("\n");
//				SymbolObject *tmp_name = Symbol_FromCStr(tmp);
//				ChainMap_SetItem(globals, tmp_name, CAR(vm->operands));
//				DECREF(tmp_name);
//				VirtualMachine_Pop(vm);
//#ifdef __DEBUG__
//
//				printf("\n");
//#endif
//
//			}
//			DECREF(d);
//			DECREF(code);
//			d = ParseDatum(l);
//		}
////	Code_Freeze(code);
//#ifdef __DEBUG__
//		fclose(fout);
//#endif
//		free(buf);
//		Lexer_Finalize(l);
//	}
//}

const char *to_string(Object *obj) {
	static char *buf = NULL;
	if (buf != NULL) {
		free(buf);
		buf = NULL;
	}
	size_t size = 0;
	FILE *f = open_memstream(&buf, &size);
	assert(f);
	if (TYPE(obj)->tp_print) {
		PRINT(obj, f);
	} else {
		fprintf(f, "type %s has no print method", TYPE(obj)->tp_name);
	}
	fclose(f);
	return buf;
}

int main() {
	TypeObjectsInit();
	Long_SmallIntsInitialize();
	GlobalSymbolsInit();
	bytecode_test();
	GlobalSymbolsFinalize();
//	repl();
//	Object *x, *y, *z;
//	LexerObject *l = Lexer_FromCStr("(quote (1 2 3))");
//	Object *d = ParseDatum(l);
//	assert(ParseArgs(d, "'(? ? ?)", &x, &y, &z) == 3);
//	PRINT(x, stdout);
//	putchar('\n');
//	PRINT(y, stdout);
//	putchar('\n');
//	FrameObject globals = {
//			1, &Frame_Type, NULL, ChainMap_NewEmpty()
//	};
//	Dict_SetItem(globals, global_symbols.quote,)

//	assert(ParseArgs(y, "(if ? ? ?)", &x, &y, &z) == 3);
//	TYPE(x)->tp_print(x, stdout);
//	putchar('\n');
//	TYPE(y)->tp_print(y, stdout);
//	putchar('\n');
//	TYPE(z)->tp_print(z, stdout);
//	putchar('\n');
//	DECREF(d);
//	Lexer_Finalize(l);
	//	TokenObject *t = NULL;
//	for (;;) {
//		Lexer_GetNextToken(&t);
//		if (t != NULL) {
//			Token_Print(t, stdout);
//			putchar('\n');
//			DECREF(t);
//			t = NULL;
//		} else {
//			break;
//		}
//	}
//	Lexer_Finalize(lexer);
	return 0;
}
