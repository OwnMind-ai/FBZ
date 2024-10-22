#include <stdio.h>
#include <stdlib.h>
#include "parser/parser.h"

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
	Parser *parser = createParser(lexer);
	
	FileToken *f = parseFile(parser);
	for(int i = 0; i < f->size; i++){
		FunctionToken *t = f->functions[i];
		printf("FUNCTION %s(", t->name);
	
		for(int j = 0; j < t->parametersLength; j++)
			printf("%s, ", t->parameters[j]);

		printf(") [\n");
		printToken(t->rangeToken->from, 1);
		printToken(t->rangeToken->to, 1);
		printToken(t->rangeToken->step, 1);
		printf("]:\n");
	
		for(int j = 0; j < t->conditionsLength; j++)
			printToken(t->conditions[j], 1);
	}

	fclose(file);
}
