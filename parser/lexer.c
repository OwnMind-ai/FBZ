#include "parser.h"
#include "tokens.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

const char *KEYWORDS[] = {
	"else", "end"
};

int isOperatorStart(char c) { return strchr("+-*/%>=", c) != NULL; }

int operatorPrecedence(char c[2]){
	if(c[0] == '>' && c[1] == '>') return 1;

	if(!isOperatorStart(c[1])){
		switch (c[0]){
			case '+': case '-': 
				return 10;
			case '/': case '*': case '%':
				return 20;
			case '=': return 5;

			default: return -1;	
		}
	}

	return -1;
}

struct Lexer {
	FILE *file;
	size_t len;
	Token *last;
};

typedef struct {
	int line;
	int column;
} FilePosition;

char fpeek(FILE *file){
	char result = fgetc(file);
	ungetc(result, file);

	return result;
}

int fpeekn(char *result, int n, Lexer *lexer){
	size_t was = ftell(lexer->file);
	if(was + n > lexer->len) return 0;
	
	char buf[n + 1];
	fgets(buf, n + 1, lexer->file);
	fseek(lexer->file, was, SEEK_SET);
	memcpy(result, buf, n);

	return 1;
}

/**
 * Calculates current position in file. Slow function, use only for error messages
 */
FilePosition fgetp(FILE *file){
	long was = ftell(file);
	rewind(file);

	FilePosition result = {0, 0};
	for(long i = 0; i <= was; i++){
		if(fgetc(file) == '\n') {
			result.line++;
			result.column = 0;
		} else {
			result.column++;
		}
	}

	result.line++; 
	result.column++;

	return result;
}

int isWhitespace(char c){ return strchr(" \r\t", c) != NULL; }

int isNotLineBreak(char c){ return c == '\n'; }

int isNumber(char c){ return '0' <= c && c <= '9'; }

int isWordPart(char c){ return c == '_' || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || isNumber(c); }

char* readWhile(int (predicate)(char), Lexer *lexer, int store){
	int len = 0;
	char *result = store ? malloc(1) : NULL; 

	while(!feof(lexer->file) && predicate(fpeek(lexer->file))){
		char next = fgetc(lexer->file);

		if(store){
			result = realloc(result, ++len + 1);
			result[len - 1] = next;
		}
	}

	if(store)
		result[len] = 0;

	return result;
}

void skipComments(Lexer *lexer){
	if(feof(lexer->file)) return;

	char peeked[3];
	int success = fpeekn(peeked, 2, lexer);
	if(!success) return;
	if(peeked[0] != '/') return;
	
	if(peeked[1] == '/'){
		readWhile(&isNotLineBreak, lexer, 0);
		fseek(lexer->file, 1, SEEK_CUR);
	} else if(peeked[1] == '*') {
		fseek(lexer->file, 2, SEEK_CUR);

		char end[3];
		while(fpeekn(end, 2, lexer) && (end[0] != '*' || end[1] != '/')){
			fseek(lexer->file, 1, SEEK_CUR);
		}
		
		// skip '*/'
		if(!feof(lexer->file))
			fseek(lexer->file, 2, SEEK_CUR);
	}
}

void skipWhitespaces(Lexer *lexer){
	readWhile(&isWhitespace, lexer, 0);
	skipComments(lexer);
	readWhile(&isWhitespace, lexer, 0);
}

Token *parseNumber(Lexer *lexer){
	Token *result = malloc(sizeof(Token));
	result->tokenType = NUMBER;
	
	char *intPart = readWhile(&isNumber, lexer, 1);
	result->number = atol(intPart);
	free(intPart);

	if(fpeek(lexer->file) == '.'){
		fseek(lexer->file, 1, SEEK_CUR);
		char* decimalPart = readWhile(&isNumber, lexer, 1);
		result->number += ((double) atol(decimalPart)) / pow(10, strlen(decimalPart));
		free(decimalPart);
	}

	return result;
}

char escapeChar(char c) {
    switch (c) {
        case 'n': return '\n'; 
        case 't': return '\t'; 
        case 'r': return '\r'; 
        case '\\': return '\\'; 
        case '\'': return '\''; 
        case '\"': return '\"';
        case '0': return '\0';  
        default: return c;      
    }
}

Token *parseString(Lexer *lexer){
	char literal = fgetc(lexer->file);
	char *result = malloc(1);
	result[0] = 0;
	int len = 0;

	char c = fgetc(lexer->file);
	while(c != literal){
		if(feof(lexer->file) || c == '\n')
			stopParsing(lexer, "Unterminated string literal");

		result = realloc(result, ++len + 1);
		result[len - 1] = c;

		c = fgetc(lexer->file);
		
		if(c == '\\')
			c = escapeChar(fgetc(lexer->file));
	}

	Token *t = malloc(sizeof(Token));
	t->tokenType = STRING;
	t->string = result;

	return t;
}

Token *parseOperator(Lexer *lexer){
	Token *result = malloc(sizeof(Token));
	result->tokenType = OPERATOR;

	char op[2];
	fpeekn(op, 2, lexer);

	if(operatorPrecedence(op) != -1){
		result->op[0] = fgetc(lexer->file);

		if(isOperatorStart(op[1]))
			result->op[1] = fgetc(lexer->file);
		else
			result->op[1] = 0;

		return result;
	}

	stopParsing(lexer, "Invalid operator '%s'", op);
}

Token *parseWord(Lexer *lexer){
	Token *token = malloc(sizeof(Token));
	char *word = readWhile(&isWordPart, lexer, 1);

	size_t n = sizeof(KEYWORDS)/sizeof(KEYWORDS[0]);
	for(size_t i = 0; i < n; i++){
		if(!strcmp(word, KEYWORDS[i])){
			token->tokenType = KEYWORD;
			token->keyword = word;

			return token;
		}
	}

	token->tokenType = VARIABLE;
	token->variable = word;
	return token;
}

Token* lparseToken(Lexer *lexer){
	skipWhitespaces(lexer);

	char peeked = fpeek(lexer->file);

	if(isNumber(peeked))
		return parseNumber(lexer);

	if(peeked == '"' || peeked == '\'')
		return parseString(lexer);

	if(strchr("()[]{}:,\n", peeked)){
		Token *p = malloc(sizeof(Token));
		p->tokenType = PUNCTUATION;
		p->punctuation = fgetc(lexer->file);

		return p;
	}

	if(isOperatorStart(peeked)){
		Token *result = parseOperator(lexer);
		peeked = fpeek(lexer->file);
		
		if((result->op[0] == '-' || result->op[0] == '+') && peeked >= '0' && peeked <= '9'){
			Token *number = parseNumber(lexer);
			if(result->op[0] == '-') number->number *= -1;
		
			return number;
		
		}

		return result;
	}

	if(isWordPart(peeked))
		return parseWord(lexer);

	stopParsing(lexer, "Invalid token '%s'", readWhile(&isWhitespace, lexer, 1));
}

Lexer* createLexer(FILE *file){
	Lexer* result = malloc(sizeof(Lexer));
	result->file = file;

	fseek(file, 0, SEEK_END);
	result->len = ftell(file);
	rewind(file);

	result->last = lparseToken(result);

	return result;
}

int leof(Lexer *lexer){
	return lexer->last == NULL;
}

Token* lpeek(Lexer *lexer){
	return lexer->last;
}

Token* lnext(Lexer *lexer){
	Token *result = lexer->last;
	skipWhitespaces(lexer);
	lexer->last = feof(lexer->file) ? NULL : lparseToken(lexer);

	return result;
}

void stopParsing(Lexer *lexer, char *format, ...){
	va_list args;
	va_start(args, format);
	char out[100];
	vsnprintf(out, 100, format, args);
	va_end(args);

	FilePosition position = fgetp(lexer->file);
	fprintf(stderr, "SyntaxError (%d, %d):\n%s\n", position.line, position.column, out);
	exit(1);
}
