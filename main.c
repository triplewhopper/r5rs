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
#include "Include/noneobject.h"
#include "Include/frameobject.h"
#include "Include/vmobject.h"
#include "library/builtins.h"


StringObject *readline() {
	char *buf = NULL;
	int c = 0;
	size_t size = 0;
	FILE *f = open_memstream(&buf, &size);
	assert(f);
	while ((c = getchar()) != '\n') fputc(c, f);
	fclose(f);
	StringObject *res = String_FromCStrN(buf, size);
	free(buf);
	return res;
}

Object *eval(Object *datum, ChainMap *globals) {
	CodeObject *code = Code_New(NULL);
	dfs(datum, code, 0, 1, NULL);
	Code_Freeze(code);
	VirtualMachineObject *vm = VirtualMachine_New(globals);
	if (VirtualMachine_Exec(vm, code) < 0) {
		DECREF(vm);
		DECREF(code);
		return NULL;
	} else {
		DECREF(code);
		assert(IS_NOT_NULL(vm->operands));
		assert(Pair_ListLength(vm->operands) == 1);
		Object *res = NewRef(CAR(vm->operands));
		DECREF(vm);
		return res;
	}
}

void load_from_file(ChainMap *globals, const char *filename) {
	LexerObject *l = Lexer_FromFile(filename);
	StringObject *bc = String_Format("%s.bytecode.txt", filename);
	FILE *fout = fopen(bc->ob_sval, "w");
	DECREF(bc);
	int count = 0;
	Object *d = ParseDatum(l);
	while (d) {
#ifdef __DEBUG__
		printf("evaluating ");
		PRINT(d, stdout);
		printf("\n");
#endif
		Object *obj = eval(d, globals);
		if (obj == NULL) {
			break;
		} else {
			if (IS_NOT_NONE(obj)) {
				printf("$%d=", ++count);
				PRINT(obj, stdout);
				printf("\n");
			}
		}
		DECREF(d);
		DECREF(obj);
		d = ParseDatum(l);
	}
	fclose(fout);
	DECREF(l);
}

LexerObject *repl_read(StringObject *first_line) {
	size_t k = 0;
	TokenObject *token = NULL;
	ArrayObject *paren_stack = Array_New(0, sizeof(char));
	LexerObject *l = Lexer_FromCStr(first_line);
	char *buf = NULL;
	size_t size = 0;
	FILE *f = open_memstream(&buf, &size);
	assert(f);
	fputs(first_line, f);
	free(first_line);
	while (1) {
		while ((token = Lexer_PeekForwardTokenBy(l, k++)) != NULL) {
			if (token->kind == L_PAREN) Array_Append(paren_stack, "(");
			if (token->kind == R_PAREN) {
				if (SIZE(paren_stack) > 0 && Array_Last(char, paren_stack) == '(') {
					Array_Remove(paren_stack, SIZE(paren_stack) - 1);
				} else {
					fprintf(stderr, "unexpected ')'.");
					DECREF(paren_stack);
					DECREF(l);
					fclose(f);
					free(buf);
					return NULL;
				}
			}
		};
		if (SIZE(paren_stack) > 0) {
			DECREF(l);
			printf(".. ");
			fflush(stdout);
			first_line = readline();
			l = Lexer_FromCStr(first_line);
			k = 0;
			fputc('\n', f);
			fputs(first_line, f);
			free(first_line);
		} else {
			break;
		}
	}
	DECREF(paren_stack);
	DECREF(l);
	fclose(f);
	l = Lexer_FromCStr(buf);
	free(buf);
	return l;
}

void repl(ChainMap *globals) {
	int count = 0;
	while (1) {
		printf("?> ");
		const char *buf = readline();
		if (!strcmp(buf, ",q")) {
			free(buf);
			gc_collect();
			break;
		} else if (!strcmp(buf, ",gc")) {
			free(buf);
			gc_collect();
			continue;;
		}

		LexerObject *l = repl_read(buf);

		for (Object *d = ParseDatum(l); d != NULL; DECREF(d), d = ParseDatum(l)) {
#ifdef __DEBUG__
			printf("evaluating ");
			PRINT(d, stdout);
			printf("\n");
#endif
			Object *obj = eval(d, globals);
			if (obj == NULL) {
				break;
			} else {
				if (IS_NOT_NONE(obj)) {
					printf("$%d=", ++count);
					PRINT(obj, stdout);
					printf("\n");
					SymbolObject *tmp_name = Symbol_Format("$%d", count);
					ChainMap_SetItem(globals, tmp_name, obj);
					DECREF(tmp_name);
				}
				DECREF(obj);
			}
		}
#ifdef __DEBUG__
		fclose(fout);
#endif
		DECREF(l);
	}
}

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
	Type_InitTypeObjects();
	gc_initialize();
	Long_SmallIntsInitialize();
	GlobalSymbolsInit();
	ChainMap *builtins = load_builtins();
	ChainMap *globals = ChainMap_NewChild(builtins, NULL);
	repl(globals);
	DECREF(globals);
	DECREF(builtins);
	gc_collect();
	GlobalSymbolsFinalize();
	gc_finalize();
	return 0;
}
