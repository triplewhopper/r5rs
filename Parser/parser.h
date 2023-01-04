#ifndef R5RS_PARSER_H
#define R5RS_PARSER_H

#include "../Lexer/lexer.h"
#include "../Lexer/token.h"



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

int ParseArgsImpl(Object *obj, Object *datum, va_list ap);

int ParseArgs(Object *obj, const char *format, ...);

#endif //R5RS_PARSER_H
