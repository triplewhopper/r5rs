#ifndef R5RS_REPL_H
#define R5RS_REPL_H

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

#endif //R5RS_REPL_H
