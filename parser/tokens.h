#ifndef TOKENS_H
#define TOKENS_H

// NOTE: \n considered to be a punctuation. If parser encounters it and it was not expected, parser must ignore it.
typedef enum {
	OPERATION, CALL,
	OPERATOR, PUNCTUATION, VARIABLE, KEYWORD, STRING, NUMBER
} TokenType;

struct Token;
typedef struct Token Token;

typedef struct {
	Token *from;
	Token *to;
	Token *step;
} RangeToken;

typedef struct {
	char op[2];
	Token *left;
	Token *right;
} OperationToken;

typedef struct {
	char *function;
	int argsLen;
	Token **args;
} CallToken;

typedef struct {
	char *name;
	int parametersLength;
	char **parameters;
	RangeToken *rangeToken;	
	int conditionsLength;
	Token **conditions;
} FunctionToken;

typedef struct {
	int size;
	FunctionToken **functions;
} FileToken;

struct Token {
	union {
		char punctuation;
		char op[2];
		char *keyword;
		char *variable;
		char *string;
		double number;
		OperationToken *operation;
		CallToken *call;
	};
	TokenType tokenType;
};

#endif
