# Ski scheme

An interpreter conforming with a subset of r5rs specification. 

This is one of the author's homeworks for [istube22 -- 情報科学基礎実験2022](https://sites.google.com/view/isutbe2022) at UTokyo.

## Table of Contents

- Install

- Usage

- Examples

## Install

`git clone git@github.com:triplewhopper/r5rs.git`

`cd r5rs`

`mkdir cmake-build`



## Usage

`r5rs [-l FILE] [-e EXPR]`

1. Options 
- `-l FILE`
  
  Load Scheme source code from FILE.

- `--` 
  Stop argument processing, and start **r5rs** in interactive mode.  

- `-h` `--help` 
  Describe command-line options and exit.

## DESCRIPTION

### Surpported Features

- Proper tail recursion
- Garbage collection (reference counting, cyclic reference detection)
- Compiled to bytecode
- Simple REPL
- Fraction and complex number

### Not supported yet

- Arbitrary precision integers
- Static analysis of source code
- Executing bytecode directly   
- Bytecode optimization
- Exceptions handling
- `quasiquote` 
- `unquote` 
- `call-with-current-continuation`

### Builtin functions

- Type predicates
  
   `pair?` `null?` `string?` `char?` `boolean?` `symbol?` `procedure?`

- pair operations
  
  `car` `cdr` `cons`

- equivalence predicates
  
  `eq?` `eqv?`   
  
  list operations

`list?` `list-length` `list-ref`

#### numeric operations

`+` `-` `*` `/` `remainder` `<` `<=` `=` `>=` `>`

#### I/O

`display` `newline` `read` `load`

#### Special Forms

`set!` `define` `lambda` `if` `cond`


