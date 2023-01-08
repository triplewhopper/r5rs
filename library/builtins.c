#include <stdarg.h>
#include <inttypes.h>
#include "builtins.h"
#include "object.h"
#include "chainmapobject.h"
#include "typeobject.h"
#include "pairobject.h"
#include "booleanobject.h"
#include "stringobject.h"
#include "symbolobject.h"
#include "procedureobject.h"
#include "noneobject.h"
#include "numberobject.h"
#include "interpreter.h"

int CheckArgc(size_t argc, size_t n) {
	if (argc < n) {
		fprintf(stderr, "too few arguments ");
		fprintf(stderr, "(expected %zu, got %zu)", n, argc);
		return -1;
	} else if (argc > n) {
		fprintf(stderr, "too many arguments ");
		fprintf(stderr, "(expected %zu, got %zu)", n, argc);
		return -1;
	}
	return 0;
}

#define CHECK_ARGC(n, func_name) \
if (CheckArgc(argc, n) != 0) {\
fprintf(stderr, "passed to procedure " func_name "\n");\
return NULL;\
}

#define TYPE_ERR_STR "in procedure %s, parameter %s: expected type %s, got %s instead\n"
#define EXPECT_TYPE(param, obj, type, proc) do{ \
    if(!IS_TYPE(obj, type)){ \
        fprintf(stderr, TYPE_ERR_STR, proc, param, TYPE(obj)->tp_name, (type).tp_name); \
        return NULL;\
    }\
}while(0)


#define EXPECT_LIST(param, obj, proc) do{ \
    if(!IS_LIST(obj)){ \
        fprintf(stderr, "in procedure %s, parameter %s: expect list, got %s\n", \
        proc, param, TYPE(obj)->tp_name); \
        return NULL;\
    }\
}while(0)

#define EXPECT_NUMBER(param, obj, proc) \
do{                            \
if(!IS_NUMBER(obj)){       \
fprintf(stderr, "in procedure %s, parameter %s: expect number, got %s\n", \
proc, param, TYPE(obj)->tp_name); \
return NULL;\
}\
}while(0)

#define EXPECT_REAL(param, obj, proc) \
do{ \
    if(!IS_REAL(obj)){ \
        fprintf(stderr, "in procedure %s, parameter %s: expect real number, got %s\n", \
        proc, param, TYPE(obj)->tp_name); \
        return NULL; \
    } \
}while(0)

#define AT_LEAST(len, at_least) if ((len) < (at_least)) {\
    fprintf(stderr, "too few arguments ");\
    fprintf(stderr, "(expected >=%d, got %zu)", (at_least), (len));\
    return NULL;\
}\


//Object *is_pair(Object *args) {
//	CHECK_ARGC(1, "(pair? obj)")
//	if (IS_TYPE(CAR(args), Pair_Type)) RETURN_TRUE;
//	RETURN_FALSE;
//}

static Object *builtin_exit(size_t argc, Object *argv[]){
	// TODO release allocated memory
	if(argc == 0){
		exit(0);
	}else{
		CHECK_ARGC(1, "(exit n)")
		EXPECT_TYPE("n", argv[0], Long_Type, "(exit n)");
		exit(AS_LONG(argv[0])->ob_val);
	}
	assert(0);
}
static Object *is_pair(size_t argc, Object *argv[]) {
	CHECK_ARGC(1, "(pair? obj)")
	if (IS_TYPE(argv[0], Pair_Type)) RETURN_TRUE;
	RETURN_FALSE;
}

static Object *is_null(size_t argc, Object *argv[]) {
	CHECK_ARGC(1, "(null? obj)")
	if (IS_NULL(argv[0])) RETURN_TRUE;
	RETURN_FALSE;
}

static Object *is_list(size_t argc, Object *argv[]) {
	CHECK_ARGC(1, "(list? obj)")
	if (IS_LIST(argv[0])) RETURN_TRUE;
	RETURN_FALSE;
}

static Object *is_string(size_t argc, Object *argv[]) {
	CHECK_ARGC(1, "(string? obj)")
	if (IS_TYPE(argv[0], String_Type)) RETURN_TRUE;
	RETURN_FALSE;
}

static Object *is_char(size_t argc, Object *argv[]) {
	CHECK_ARGC(1, "(char? obj)")
	if (IS_TYPE(argv[0], Char_Type)) RETURN_TRUE;
	RETURN_FALSE;
}

static Object *is_boolean(size_t argc, Object *argv[]) {
	CHECK_ARGC(1, "(boolean? obj)")
	if (IS_TYPE(argv[0], Boolean_Type)) RETURN_TRUE;
	RETURN_FALSE;
}

static Object *is_symbol(size_t argc, Object *argv[]) {
	CHECK_ARGC(1, "(symbol? obj)")
	if (IS_TYPE(argv[0], Symbol_Type)) RETURN_TRUE;
	RETURN_FALSE;
}

static Object *is_procedure(size_t argc, Object *argv[]) {
	CHECK_ARGC(1, "(procedure? obj)")
	if (IS_TYPE(argv[0], Procedure_Type)) RETURN_TRUE;
	RETURN_FALSE;
}

static Object *car(size_t argc, Object *argv[]) {
	CHECK_ARGC(1, "(car p)")
	EXPECT_TYPE("p", argv[0], Pair_Type, "(car p)");
	return NewRef(CAR(argv[0]));
}

static Object *cdr(size_t argc, Object *argv[]) {
	CHECK_ARGC(1, "(cdr p)")
	EXPECT_TYPE("p", argv[0], Pair_Type, "(cdr p)");
	return NewRef(CDR(argv[0]));
}

static Object *cons(size_t argc, Object *argv[]) {
	CHECK_ARGC(2, "(cons obj1 obj2)");
	return CONS(argv[0], argv[1]);
}

static Object *eq(size_t argc, Object *argv[]) {
	CHECK_ARGC(2, "(eq? obj1 obj2)");
	if (TYPE(argv[0])->tp_cmp->cmp_eq) {
		if (TYPE(argv[0])->tp_cmp->cmp_eq(argv[0], argv[1]))
			RETURN_TRUE;
	} else {
		if (TYPE(argv[0])->tp_base->tp_cmp->cmp_eq(argv[0], argv[1]))
			RETURN_TRUE;
	}
	RETURN_FALSE;
}

static Object *eqv(size_t argc, Object *argv[]) {
	CHECK_ARGC(2, "(eqv? obj1 obj2)");
	if (TYPE(argv[0])->tp_cmp->cmp_eqv(argv[0], argv[1]))
		RETURN_TRUE;
	RETURN_FALSE;
}

static Object *list_length(size_t argc, Object *argv[]) {
	CHECK_ARGC(1, "(list-length lst)")
	EXPECT_LIST("lst", argv[0], "(list-length lst)");
	i64 ans = 0;
	for (Object *t = argv[0]; IS_NOT_NULL(t); t = CDR(t)) {
		ans++;
	}
	return AS_OBJECT(Long_From_i64(ans));
}

static Object *list_ref(size_t argc, Object *argv[]) {
	CHECK_ARGC(2, "(list-ref lst index)")
	EXPECT_LIST("lst", argv[0], "(list-ref lst index)");
	EXPECT_TYPE("index", argv[1], Long_Type, "(list-ref lst index)");
	Object *lst = argv[0];
	i64 const index = AS_LONG(argv[1])->ob_val;
	const size_t n = Pair_ListLength(lst);
	if (n <= index) {
		fprintf(stderr, "in procedure %s: index out of range: %lld\n", "(list-ref lst index)", index);
		return NULL;
	}

	for (size_t i = 0; i < index; ++i) {
		lst = CDR(lst);
	}
	return NewRef(CAR(lst));
}

static Object *num_add(size_t argc, Object *argv[]) {
	if (argc == 0) return AS_OBJECT(Long_From_i64(0));
	for (size_t i = 0; i < argc; ++i) {
		if (!IS_NUMBER(argv[i])) {
			fprintf(stderr, "in procedure (+ ...), parameter $%zu: "
							"expected number?, got %s instead\n", i, TYPE(argv[i])->tp_name);
			return NULL;
		}
	}
	Object *ans = NewRef(argv[0]);
	Object *tmp;
	for (size_t i = 1; i < argc; ++i) {
		MOVE_SET(tmp, ans, Number_Add(ans, argv[i]));
	}
	return ans;
}


static Object *num_mul(size_t argc, Object *argv[]) {
	Object *ans = AS_OBJECT(Long_From_i64(1));
	if (argc == 0) return ans;
	for (size_t i = 0; i < argc; ++i) {
		if (!IS_NUMBER(argv[i])) {
			fprintf(stderr, "in procedure (* ...), parameter $%zu: "
							"expected number?, got %s instead\n", i, TYPE(argv[i])->tp_name);
			return NULL;
		}
	}
	Object *tmp;

	for (size_t i = 0; i < argc; ++i) {
		MOVE_SET(tmp, ans, Number_Mul(ans, argv[i]));
	}
	return ans;
}

static Object *num_sub(size_t argc, Object *argv[]) {
	AT_LEAST(argc, 1);
	for (size_t i = 0; i < argc; ++i) {
		if (!IS_NUMBER(argv[i])) {
			fprintf(stderr, "in procedure (- ...), parameter $%zu: "
							"expected number?, got %s instead\n", i, TYPE(argv[i])->tp_name);
			return NULL;
		}
	}
	Object *ans = AS_OBJECT(Long_From_i64(0));
	Object *tmp;
	if (argc == 1) {
		MOVE_SET(tmp, ans, Number_Sub(ans, argv[0]));
	} else {
		MOVE_SET(tmp, ans, NewRef(argv[0]));
	}

	for (size_t i = 1; i < argc; ++i) {
		MOVE_SET(tmp, ans, Number_Sub(ans, argv[i]));
	}
	return ans;
}

static Object *num_div(size_t argc, Object *argv[]) {
	AT_LEAST(argc, 1);
	for (size_t i = 0; i < argc; ++i) {
		if (!IS_NUMBER(argv[i])) {
			fprintf(stderr, "in procedure (/ ...), parameter $%zu: "
							"expected number?, got %s instead\n", i, TYPE(argv[i])->tp_name);
			return NULL;
		}
	}
	Object *ans = AS_OBJECT(Long_From_i64(1));
	Object *tmp;
	if (argc == 1) {
		MOVE_SET(tmp, ans, Number_Div(ans, argv[0]));
	} else {
		MOVE_SET(tmp, ans, NewRef(argv[0]));
	}
	for (size_t i = 1; i < argc; ++i) {
		MOVE_SET(tmp, ans, Number_Div(ans, argv[i]));
	}
	return ans;
}

static Object *num_rem(size_t argc, Object *argv[]) {
	CHECK_ARGC(2, "(reminder n1 n2)")
	if (!IS_INTEGER(argv[0])) {
		fprintf(stderr, "in procedure (remainder n1 n2), parameter n1: "
						"expected integer?, got %s instead\n", TYPE(argv[0])->tp_name);
		return NULL;

	}
	if (!IS_INTEGER(argv[1])) {
		fprintf(stderr, "in procedure (remainder n1 n2), parameter n2: "
						"expected integer?, got %s instead\n", TYPE(argv[1])->tp_name);
		return NULL;
	}
	Object *n1 = NUMBER_METHODS(argv[0])->nb_long(argv[0]);
	Object *n2 = NUMBER_METHODS(argv[1])->nb_long(argv[1]);
	Object *res = AS_OBJECT(Long_Remainder(AS_LONG(n1), AS_LONG(n2)));
	if (IS_INEXACT(argv[0]) || IS_INEXACT(argv[1])) {
		Object *tmp;
		MOVE_SET(tmp, res, NUMBER_METHODS(res)->nb_to_inexact(res));
	}
	DECREF(n1), DECREF(n2);
	return res;
}

#define MAKE_NUM_CMP(kind, name, method)\
static Object *num_##kind(size_t argc, Object *argv[]){ \
    AT_LEAST(argc, 2);\
    for (size_t i = 0; i < argc; ++i) {\
        if (!IS_NUMBER(argv[i])) { \
            fprintf(stderr, "in procedure (" name " _ _ ...), parameter $%zu: " \
                "expected real?, got %s instead\n", i, TYPE(argv[i])->tp_name); \
            return NULL; \
        } \
    }\
    for (size_t i = 0 ; i + 1 < argc; ++i) {\
        if (!method(argv[i], argv[i + 1]))\
            RETURN_FALSE;\
    }\
    RETURN_TRUE;\
}

MAKE_NUM_CMP(lt, "<", Number_LT)

MAKE_NUM_CMP(le, "<=", Number_LE)

MAKE_NUM_CMP(eq, "=", Number_EQ)

MAKE_NUM_CMP(ge, ">=", Number_GE)

MAKE_NUM_CMP(gt, ">", Number_GT)
//Object *logical_and(Object *args){
//
//}
//
//Object *logical_or(Object *args){
//
//}
//
//Object *logical_not(Object *args){
//
//}

static Object *display(size_t argc, Object *argv[]) {
	AT_LEAST(argc, 1);
	for (size_t i = 0; i < argc; ++i) {
		if (TYPE(argv[i])->tp_print)
			PRINT(argv[i], stdout);
		else printf("<%s object @ %p>", TYPE(argv[i])->tp_name, argv[i]);
		if (i + 1 < argc) {
			printf(" ");
		}
	}
	RETURN_NONE;
}

static Object *newline(size_t argc, Object *argv[]) {
	printf("\n");
	RETURN_NONE;
}

static Object *load(size_t argc, Object *argv[]) {
	CHECK_ARGC(1, "(load file-name)")
	assert(0);
}


static struct builtin_funcs_kvpair_t {
	const char *func_name;

	// receives borrowed ref's, returns a new ref.
	Object *(*func)(size_t, Object *[]);
} builtin_funcs[] = {
//		{"number?",     IsNumber},
//		{"complex?",    IsComplex},
//		{"real?",       IsReal},
//		{"rational?",   IsRational},
//		{"integer?",    IsInteger},
		{"exit",        builtin_exit},
		{"pair?",       is_pair},
		{"null?",       is_null},
		{"list?",       is_list},
		{"string?",     is_string},
		{"char?",       is_char},
		{"boolean?",    is_boolean},
		{"symbol?",     is_symbol},
		{"procedure?",  is_procedure},
		{"car",         car},
		{"cdr",         cdr},
		{"cons",        cons},
		{"eq?",         eq},
		{"eqv?",        eqv},
		{"list-length", list_length},
		{"list-ref",    list_ref},
		{"+",           num_add},
		{"-",           num_sub},
		{"*",           num_mul},
		{"/",           num_div},
		{"remainder",   num_rem},
		{"<",           num_lt},
		{"<=",          num_le},
		{"=",           num_eq},
		{">=",          num_ge},
		{">",           num_gt},
		{"display",     display},
		{"newline",     newline},
		{"load",        load},
		{NULL, NULL}
};

ChainMap *load_builtins() {
	ChainMap *res = ChainMap_NewEmpty();
	for (typeof(&builtin_funcs[0]) t = builtin_funcs; t->func_name != NULL; t++) {
		SymbolObject *key = Symbol_FromCStr(t->func_name);
		ProcedureObject *value = Procedure_FromCFunction(key->name, t->func);
		ChainMap_SetItem(res, key, AS_OBJECT(value));
		DECREF(key), DECREF(value);
	}
	return res;
}
