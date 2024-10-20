#ifndef TOKENS_H
#define TOKENS_H

// NOTE: \n considered to be a punctuation. If parser encounters it and it was not expected, parser must ignore it.
typedef enum {
	OPERATOR, PUNCTUATION, VARIABLE, KEYWORD, STRING, NUMBER
} TokenType;

typedef struct {
	TokenType tokenType;
	union {
		char punctuation;
		char op[2];
		char *keyword;
		char *variable;
		char *string;
		double number;
	};
} Token;

#endif
