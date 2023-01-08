# r5rs

An interpreter conforming with a subset of r5rs specification. 

This is one of the author's homeworks for [istube22 -- 情報科学基礎実験2022](https://sites.google.com/view/isutbe2022) at UTokyo.

## Table of Contents

- Install

- Usage

- Examples

## Install

`cd r5rs`

`cmake .; make`


## Usage

`r5rs [FILE ...]`

If there are file arguments, then execute them sequentially, and exit.

If there is no argument, launch the REPL.

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
  
  `eq?` `eqv?` `equal?`

  `equal?` is defined in `r5rs/samples/s12.scm`. 
  
- list operations

  `list?` `list-length` `list-ref`

- numeric operations

  `+` `-` `*` `/` `remainder` `<` `<=` `=` `>=` `>`

#### I/O

`display` `newline`

#### Special Forms

`set!` `define` `lambda` `if` `cond`


## Examples
```shell
$ ./r5rs ./samples/s1.scm
3
$ ./r5rs ./samples/s3.scm
99
$ ./r5rs ./samples/s4.scm
89
$ ./r5rs ./samples/s6.scm
701408733
$ ./r5rs ./samples/s7.scm
6.00010000001205
1.41421356245306
$ ./r5rs ./samples/s8.scm
9
$ ./r5rs ./samples/s9.scm
3
3
3
$ ./r5rs ./samples/s10.scm
11
$ ./r5rs ./samples/s11.scm
(5 4 3 2 1)
$ ./r5rs ./library/logicals.scm ./samples/s12.scm
(2 . "Two")
#f
$ ./r5rs ./samples/s13.scm
(5 4 3 2 1)
$ ./r5rs ./library/logicals.scm ./samples/s15.scm
./samples/s15.scm: No such file or directory
$ ./r5rs ./samples/s14.scm
(1 . 2)
(7/2 . 31/12)
$ ./r5rs ./samples/s18.scm
10
$ ./r5rs ./library/arithmetics.scm ./samples/s20.scm
10.0000000001074
1
20.0000000002798
2
30.0000000000001
1
$ 
```