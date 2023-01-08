#ifndef R5RS_REPL_H
#define R5RS_REPL_H

#include <signal.h>
#include "codegen.h"
#include "io.h"

#define PROMPT "?> "

//static volatile int ctrl_c_pressed = 0;
//static void INTHandler(int sig){
//	ctrl_c_pressed = 1;
//	printf("keyboard interrupt\n");
//
//}

LexerObject *repl_read(FILE *input) {
	size_t k = 0;
	TokenObject *token = NULL;
	StringObject *lines = String_FromCStr("");
	StringObject *line = NULL, *tmp;
	ArrayObject *paren_stack = Array_New(0, sizeof(char));
//	PRINT(line, f);
//	CLEAR(line);
//	char *buf = NULL;
//	size_t size = 0;
	while (1) {
		k = 0;
		line = readline(stdin);
		MOVE_SET(tmp, lines, String_Add(lines, line));
		FILE *f = fmemopen(line->ob_sval, SIZE(line), "r");
		LexerObject *l = Lexer_FromStream(f, "stdin");

		while ((token = Lexer_PeekForwardTokenBy(l, k++)) != NULL) {
			if (token->kind == L_PAREN) Array_Append(paren_stack, "(");
			else if (token->kind == R_PAREN) {
				if (SIZE(paren_stack) > 0 && Array_Last(char, paren_stack) == '(') {
					Array_Remove(paren_stack, SIZE(paren_stack) - 1);
				} else {
					fprintf(stderr, "unexpected ')'.");
					DECREF(paren_stack);
					DECREF(l);
					DECREF(line);
					return NULL;
				}
			}
		};
		DECREF(l);
		DECREF(line);
		if (SIZE(paren_stack) > 0) {
			printf(".. ");
			fflush(stdout);
		} else {
			break;
		}
	}
	DECREF(paren_stack);

	LexerObject *res = Lexer_FromStream(fmemopen(lines->ob_sval, SIZE(lines), "r"), "stdin");
	k = 0;
	while (Lexer_PeekForwardTokenBy(res, k++) != NULL);
	DECREF(lines);
	return res;
}

Object *repl_eval(Object *datum, R5RS *r) {
	CodeObject *code = codegen(datum, 0, 1, NULL);
	VirtualMachineObject *vm = VirtualMachine_New(r->globals);
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

void repl_print(R5RS *r, int *output_no, Object *obj) {
	if (IS_NOT_NONE(obj)) {
		printf("$%d=", ++*output_no);
		PRINT(obj, stdout);
		printf("\n");
		SymbolObject *output_name = Symbol_Format("$%d", *output_no);
		R5RS_SetGlobalItem(r, output_name, obj);
		DECREF(output_name);
	}
}

void repl_end(R5RS *r) {
	exit(0);
}

void repl_loop(R5RS *r) {
	int input_no = 0;
	int output_no = 0;

//	signal(SIGINT, INTHandler);

	while (1) {
		wait_for_input:
		printf(PROMPT);
		fflush(stdout);
//		if (!String_EqualCStyleStr(line, ",q")) {
//			DECREF(line);
//			gc_collect();
//			break;
//		} else if (!String_EqualCStyleStr(line, ",gc")) {
//			DECREF(line);
//			gc_collect();
//			continue;
//		}

		LexerObject *l = repl_read(stdin);

		for (Object *d = ParseDatum(l); d != NULL; DECREF(d), d = ParseDatum(l)) {
//			if(ctrl_c_pressed) {
//				printf("keyboard interrupt\n");
//				ctrl_c_pressed = 0;
//				goto wait_for_input;
//			}
#ifdef __DEBUG__
			printf("evaluating ");
			PRINT(d, stdout);
			printf("\n");
#endif
			++input_no;
			Object *obj = repl_eval(d, r);
			if (obj == NULL) {
				DECREF(d);
				DECREF(l);
				goto end;
			} else {
				repl_print(r, &output_no, obj);
				DECREF(obj);
			}
		}
//		if(ctrl_c_pressed){
//			printf("keyboard interrupt\n");
//			ctrl_c_pressed = 0;
//		}
#ifdef __DEBUG__
		fclose(fout);
#endif
		DECREF(l);
	}
	end:
	repl_end(r);
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

#endif //R5RS_REPL_H
