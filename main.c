#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include "lexer.h"
#include "parser.h"
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
#include "builtins.h"
#include "interpreter.h"
#include "repl.h"
#include "io.h"


void load_from_file(R5RS *r, const char *filename) {
	FILE *f = fopen(filename, "r");
	if(f==NULL){
		perror(filename);
		exit(EXIT_FAILURE);
	}
	LexerObject *l = Lexer_FromStream(f, filename);
//	StringObject *bc = String_Format("%s.bytecode.txt", filename);
//	FILE *fout = fopen(bc->ob_sval, "w");
//	DECREF(bc);
	int count = 0;
	Object *d = ParseDatum(l);
	while (d) {
#ifdef __DEBUG__
		printf("evaluating ");
		PRINT(d, stdout);
		printf("\n");
#endif
		Object *obj = repl_eval(d, r);
		if (obj == NULL) {
			break;
		} else {
			if (IS_NOT_NONE(obj)) {
//				printf("$%d=", ++count);
				PRINT(obj, stdout);
				printf("\n");
			}
		}
		DECREF(d);
		DECREF(obj);
		d = ParseDatum(l);
	}
//	fclose(fout);
	DECREF(l);
}


int main(int argc, char *argv[]) {
	type_init();
	gc_init();
	smallints_init();
	global_symbols_init();
	R5RS *r = R5RS_New();
	if (argc > 1) {
		for(size_t i = 1; i<argc;++i){
			load_from_file(r, argv[i]);
		}
	} else {
		repl_loop(r);
	}
	R5RS_Dealloc(r);
	gc_collect();
	global_symbols_finalize();
	gc_finalize();
	return 0;
}
