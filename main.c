#include <stdio.h>
#include <stdlib.h>
#include "parser/parser.h"
#include "runtime/runtime.h"

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
	start(f);

	fclose(file);
}
