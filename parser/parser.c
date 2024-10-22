#include "parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct Parser {
	Lexer *lexer;
};

Parser *createParser(Lexer *lexer){
	Parser *result = malloc(sizeof(Parser));
	result->lexer = lexer;

	return result;
}

int skipLineBreaks(Parser *parser){
	Token *t = lpeek(parser->lexer);
	if(t->tokenType != PUNCTUATION || t->punctuation != '\n') return 0;

	do {
		freeToken(lnext(parser->lexer));
		if(leof(parser->lexer)) return 1;
		t = lpeek(parser->lexer);
	} while(t->tokenType == PUNCTUATION && t->punctuation == '\n');

	return 1;
}

Token *plnext(Parser *parser){
	Token *t = lnext(parser->lexer);
	if(t->tokenType == PUNCTUATION)
		stopParsing(parser->lexer, "Unexpected 'c'", t->punctuation);

	return t;
}

int delimiter(Token *buf[], Parser *parser, char start, char delimiter, char end, Token *function(Parser*), int skipBreaks){
	int len = 0;
	Token *next = lnext(parser->lexer);
	if(next->tokenType != PUNCTUATION && next->punctuation != start)
		stopParsing(parser->lexer, "Invalid expression, '%c' expected", start);

	while(lpeek(parser->lexer)->tokenType != PUNCTUATION || lpeek(parser->lexer)->punctuation != end){
		if(skipBreaks) skipLineBreaks(parser);
		buf[len++] = function(parser);
		if(skipBreaks) skipLineBreaks(parser);

		if(leof(parser->lexer))
			stopParsing(parser->lexer, "Unclosed '%c'", start);
		
		freeToken(next);
		next = lnext(parser->lexer);
		if(next->tokenType != PUNCTUATION || (next->punctuation != delimiter && next->punctuation != end)){
			stopParsing(parser->lexer, "Expected '%c'", delimiter);
		} else if(next->punctuation == end){
			freeToken(next);
			return len;
		}
	}

	freeToken(lnext(parser->lexer));

	return len;
}

Token *parseExpression(Parser *parser);

CallToken *parseCall(char *name, Parser *parser){
	CallToken *call = malloc(sizeof(CallToken));
	call->function = name;

	Token *buff[256];
	call->argsLen = delimiter(buff, parser, '(', ',', ')', &parseExpression, 0);
	call->args = calloc(call->argsLen, sizeof(Token*));
	memmove(call->args, buff, call->argsLen * sizeof(Token*));

	return call;
}

Token *parseToken(Parser *parser){
	Token *curr = lnext(parser->lexer);

	if(curr->tokenType == PUNCTUATION && curr->punctuation == '('){
		Token *t = parseExpression(parser);
		Token *end = lnext(parser->lexer);
		if(end->tokenType != PUNCTUATION || end->punctuation != ')')
			stopParsing(parser->lexer, "Unclosed '('");

		return t;
	}

	Token *next = lpeek(parser->lexer);
	if(curr->tokenType == VARIABLE && next->tokenType == PUNCTUATION && next->punctuation == '('){
		CallToken *call = parseCall(curr->variable, parser);
		Token *t = malloc(sizeof(Token));
		t->tokenType = CALL;
		t->call = call;

		return t;
	}

	if(curr->tokenType == PUNCTUATION)
		stopParsing(parser->lexer, "Unexpected token '%c'", curr->punctuation);

	return curr;
}

Token *buildExpressionTree(Token *prev, Parser *parser, int precedence){
	Token *peeked = lpeek(parser->lexer);
	if(peeked->tokenType == OPERATOR && operatorPrecedence(peeked->op) > precedence){
		OperationToken *operation = malloc(sizeof(OperationToken));
		memmove(operation->op, peeked->op, 2);
		freeToken(lnext(parser->lexer));
		operation->left = prev;
		operation->right = buildExpressionTree(parseToken(parser), parser, operatorPrecedence(operation->op));

		Token *t = malloc(sizeof(Token));
		t->tokenType = OPERATION;
		t->operation = operation;
		
		return buildExpressionTree(t, parser, precedence);
	}

	return prev;
}

Token *parseExpression(Parser *parser){
	return buildExpressionTree(parseToken(parser), parser, 0);
}

FunctionToken *parseFunction(Parser *parser){
	FunctionToken *token = malloc(sizeof(FunctionToken));
	
	Token *nameToken = lnext(parser->lexer);
	if(nameToken->tokenType != VARIABLE) 
		stopParsing(parser->lexer, "Invalid function name");

	token->name = nameToken->variable;

	Token *buffer[256];
	token->parametersLength = delimiter(buffer, parser, '(', ',', ')', &plnext, 1);
	token->parameters = calloc(token->parametersLength, sizeof(char*));
	for(int i = 0; i < token->parametersLength; i++){
		Token *t = buffer[i];
		if(t->tokenType != VARIABLE)
			stopParsing(parser->lexer, "Invalid parameter");

		token->parameters[i] = t->variable;
		free(t);
	}

	int rn = delimiter(buffer, parser, '[', ',', ']', &parseExpression, 1);
	if(rn < 2 || rn > 3) stopParsing(parser->lexer, "Invalid range");	
	RangeToken *r = malloc(sizeof(RangeToken));
	r->from = buffer[0];
	r->to = buffer[1];
	if(rn < 3){
		Token *num = malloc(sizeof(Token));
		num->tokenType = NUMBER;
		num->number = 1;
		buffer[2] = num;
	}
	r->step = buffer[2];
	token->rangeToken = r;

	Token *n = lnext(parser->lexer);
	if(n->tokenType != PUNCTUATION || n->punctuation != ':')
		stopParsing(parser->lexer, "Expected ':'");
	freeToken(n);

	skipLineBreaks(parser);
	token->conditionsLength = 0;
	token->conditions = NULL;
	
	while(lpeek(parser->lexer)->tokenType != KEYWORD || strcmp(lpeek(parser->lexer)->keyword, "end")){
		token->conditions = realloc(token->conditions, ++token->conditionsLength * sizeof(Token*));
		token->conditions[token->conditionsLength - 1] = parseExpression(parser);

		if(!skipLineBreaks(parser)){ 
			stopParsing(parser->lexer, "New line expected");
		}
	}

	freeToken(lnext(parser->lexer));   // skips 'end'

	return token;
}

FileToken *parseFile(Parser *parser){
	FileToken *file = malloc(sizeof(FileToken));
	file->functions = NULL;
	file->size = 0;

	while(!leof(parser->lexer)){
		skipLineBreaks(parser);
		FunctionToken *function = parseFunction(parser);

		file->functions = realloc(file->functions, ++file->size * sizeof(FunctionToken*));
		file->functions[file->size - 1] = function;
		skipLineBreaks(parser);
	}

	return file;
}

void printToken(Token *t, int leftpad){
	for(int i = 0; i < leftpad; i++)
		printf("  ");

	switch(t->tokenType){
		case STRING:
			printf("STRING: %s\n", t->string);
			break;
		case NUMBER:
			printf("NUMBER: %f\n", t->number);
			break;
		case OPERATOR:
			printf("OPERATOR: %c%c\n", t->op[0], t->op[1]);
			break;
		case PUNCTUATION:
			printf("PUNCTUATION: %c\n", t->punctuation);
			break;
		case VARIABLE:
			printf("VARIABLE: %s\n", t->variable);
			break;
		case KEYWORD:
			printf("KEYWORD: %s\n", t->keyword);
			break;
		case OPERATION:
			printf("OPERATION '%s':\n", t->operation->op);
			printToken(t->operation->left, leftpad + 1);
			printToken(t->operation->right, leftpad + 1);
			break;
		case CALL:
			printf("CALL '%s':\n", t->call->function);
			for(int i = 0; i < t->call->argsLen; i++)
				printToken(t->call->args[i], leftpad + 1);
			break;
		default: 
			printf("UNDEFINED");
			break;
	}
}

void freeToken(Token *t){
	switch(t->tokenType){
		case STRING: free(t->string); break;
		case VARIABLE: free(t->variable); break;
		case KEYWORD: free(t->keyword); break;
		case OPERATION:
			freeToken(t->operation->left);
			freeToken(t->operation->right);
			free(t->operation);
			break;
		case CALL:
			for(int i = 0; i < t->call->argsLen; i++)
				freeToken(t->call->args[i]);
			free(t->call->function);
			free(t->call);
			break;
		case OPERATOR: case NUMBER: case PUNCTUATION: break;
	}

	free(t);
}
