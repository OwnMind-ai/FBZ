#ifndef PARSER_H
#define PARSER_H

#include "tokens.h"
#include <stdio.h>

struct Lexer;
typedef struct Lexer Lexer;

Lexer *createLexer(FILE *file);
Token *lnext(Lexer *lexer);
Token *lpeek(Lexer *lexer);
int leof(Lexer *lexer);
void stopParsing(Lexer *lexer, char *format, ...) __attribute__((noreturn));

int operatorPrecedence(char[2]);

struct Parser;
typedef struct Parser Parser;

Parser *createParser(Lexer *lexer);
FileToken *parseFile(Parser *parser);
void printToken(Token *t, int leftpad);
void freeToken(Token *t);

#endif
