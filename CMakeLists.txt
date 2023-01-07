cmake_minimum_required(VERSION 3.19)
project(r5rs C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_FLAGS -Wall)

add_executable(r5rs
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
        Objects/arrayobject.c
        Objects/dictobject.c
        Objects/chainmap.c
        Objects/procedureobject.c
        Objects/frameobject.c
        Objects/vmobject.c
        Objects/codeobject.c
        library/builtins.c
        library/string2number.c
        Lexer/token.c
        Lexer/lexer.c
        Parser/parser.c
        main.c)

add_executable(lexer_test
        Lexer/lexer.c
        Lexer/token.c
        Lexer/lexer_test.c
        Objects/object.c
        Objects/stringobject.c)

add_executable(array_test Objects/object.c Objects/arrayobject.c Tests/arrayobject_test.c)
