#ifndef PARSER_H
#define PARSER_H

#include "tokens.h"
#include <stdio.h>

struct Lexer;
typedef struct Lexer Lexer;

Lexer* createLexer(FILE *file);
Token* lnext(Lexer *lexer);
Token* lpeek(Lexer *lexer);
int leof(Lexer *lexer);

int operatorPrecedence(char[2]);

#endif
