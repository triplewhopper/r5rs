cmake_minimum_required(VERSION 3.19)
project(r5rs C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_FLAGS -Wall)

add_executable(r5rs
#        Include/typedefs.h
#        Include/object.h
#        Include/typeobject.h
#        Include/floatobject.h
#        Include/complexobject.h
#        Include/longobject.h
#        Include/fractionobject.h
#        Include/numberobject.h
#        Include/booleanobject.h
#        Include/pairobject.h
#        Include/charobject.h
#        Include/stringobject.h
#        Include/vectorobject.h
#        Include/symbolobject.h
#        Include/noneobject.h
#        Include/dictobject.h
#        Include/chainmapobject.h
#
        Objects/object.c
        Objects/typeobject.c
        Objects/floatobject.c
        Objects/complexobject.c
        Objects/longobject.c
        Objects/fractionobject.c
        Objects/numberobject.c
        Objects/booleanobject.c
        Objects/pairobject.c
        Objects/charobject.c
        Objects/stringobject.c
        Objects/fractionobject.c
        Objects/vectorobject.c
        Objects/symbolobject.c
        Objects/noneobject.c
        Objects/dictobject.c
        library/builtins.h
        library/builtins.c
#        Lexer/lexer.h
#        Lexer/token.h
        Lexer/token.c
        main.c
        Parser/parser.c
        Lexer/lexer.c
#        library/string2number.h
        library/string2number.c
#        Include/procedureobject.h
        Objects/procedureobject.c
#        library/builtins.h
        Objects/frameobject.c
        Include/vmobject.h
        Objects/vmobject.c
#        Include/codeobject.h
        Objects/codeobject.c
        Objects/chainmap.c
#        Include/arrayobject.h
        Objects/arrayobject.c)

add_executable(lexer_test
        Lexer/lexer.h
        Lexer/lexer.c
        Lexer/token.h
        Lexer/token.c
        Lexer/lexer_test.c
        Objects/object.c
        Objects/stringobject.c)

add_executable(array_test Objects/object.c Objects/arrayobject.c Tests/arrayobject_test.c)
