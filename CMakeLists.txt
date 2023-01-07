cmake_minimum_required(VERSION 3.19)
project(r5rs C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_FLAGS -Wall)

include_directories(Includes)

aux_source_directory(Objects src_list)
aux_source_directory(Lexer src_list)
aux_source_directory(Parser src_list)
aux_source_directory(library src_list)

add_executable(r5rs main.c ${src_list})

#add_executable(array_test Objects/object.c Objects/arrayobject.c Tests/arrayobject_test.c)
