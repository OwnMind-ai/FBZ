#include <stdint.h>
#include <stdio.h>
#include "parser/parser.h"

void printToken(Token *t){
	switch(t->tokenType){
		case STRING:
			printf("STRING: %s\n", t->string);
			break;
		case NUMBER:
			printf("NUMBER: %f\n", t->number);
			break;
		case OPERATOR:
			printf("OPERATOR: %s\n", t->op);
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
	}
}

int main(int argc, char *argv[]){
	if(argc != 2){
		fprintf(stderr, "No path to source was specified. Example: ./fbz main.fbx\n");
		return 1;
	}

	char *path = argv[1];
	FILE *file = fopen(path, "r");

	if(file == NULL){
		fprintf(stderr, "No file named '%s'\n", path);
		return 1;
	}

	Lexer *lexer = createLexer(file);
	printf("start\n");
	while(!leof(lexer)){
		printToken(lnext(lexer));
	}

	fclose(file);
}
