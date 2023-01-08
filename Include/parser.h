#ifndef R5RS_PARSER_H
#define R5RS_PARSER_H

#include "lexer.h"
#include "tokenobject.h"



Object *ParseDatum(LexerObject *);

Object *ParseSimpleDatum(LexerObject *);

Object *ParseCompoundDatum(LexerObject *);

Object *ParseList(LexerObject *);

Object *ParseVector(LexerObject *);

Object *ParseIf();

Object *ParseQuote();

Object *ParseSetValue();

Object *ParsePair();

Object *ParseExpression();

Object *ParseLiteral();

Object *ParseProcCall();

Object *ParseLambda();

Object *read(const char *);


#endif //R5RS_PARSER_H
