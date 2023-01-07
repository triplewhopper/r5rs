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
#include "library/io.h"

static ChainMap *globals;
static ChainMap *builtins;



void load_from_file(const char *filename) {
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



ChainMap *load_globals() {
	return ChainMap_NewChild(builtins, NULL);
}

int main() {
	type_init();
	gc_init();
	smallints_init();
	global_symbols_init();
	builtins = load_builtins();
	globals = load_globals();

	repl(globals);
	DECREF(globals);
	DECREF(builtins);
	gc_collect();
	global_symbols_finalize();
	gc_finalize();
	return 0;
}
