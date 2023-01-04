#ifndef R5RS_TYPEDEFS_H
#define R5RS_TYPEDEFS_H

#include <stdint.h>
#include <stdio.h>

typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;
typedef int64_t shash_t;
typedef struct object Object;
typedef struct type_object TypeObject;
typedef struct complex_object ComplexObject;
typedef struct float_object FloatObject;
typedef struct long_object LongObject;
typedef struct fraction_object FractionObject;
typedef struct boolean_object BooleanObject;
typedef struct char_object CharObject;
typedef struct string_object StringObject;
typedef struct vector_object VectorObject;
typedef struct pair_object PairObject;
typedef struct procedure_object ProcedureObject;
typedef struct symbol_object SymbolObject;
typedef struct dict_object DictObject;
typedef struct dict_entry_object DictNode;
typedef struct chain_map_object ChainMap;
typedef struct none_object NoneObject;
typedef struct code_object CodeObject;
typedef struct frame_object FrameObject;
typedef struct number_methods NumberMethods;
typedef struct mapping_methods MappingMethods;
typedef struct cmp_methods CompareMethods;
typedef struct lexer_object LexerObject;
typedef struct virtual_machine_object VirtualMachineObject;

typedef void (*print_proc)(Object *, FILE *);

typedef void (*dealloc_proc)(Object *);

typedef void (*visit_proc)(Object *);

typedef Object *CFunction(size_t, Object *[]);

typedef Object *(*binaryfunc)(Object *, Object *);

typedef Object *(*unaryfunc)(Object *);

typedef int (*int_unaryfunc)(Object *);

typedef int (*int_binaryfunc)(Object *, Object *);

#endif //R5RS_TYPEDEFS_H
