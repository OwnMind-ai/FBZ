#include "runtime.h"
#include <stdlib.h>
#include <string.h>

struct RuntimeStack{
	StackElement **elements;
	FileToken *fileToken;
	int size;
};

RuntimeStack *rsCreate(FileToken *fileToken){
	RuntimeStack *result = malloc(sizeof(RuntimeStack));
	result->size = 0;
	result->elements = NULL;
	result->fileToken = fileToken;

	return result;
}

StackElement *rsCreateElement(char *functionName, char *name, StackElementType type, void *value){
	StackElement *result = malloc(sizeof(StackElement));
	result->name = name;
	result->functionName = functionName;
	result->type = type;
	result->value = value;

	return result;
}

void rsAdd(RuntimeStack *stack, StackElement *element){
	stack->elements = realloc(stack->elements, ++stack->size * sizeof(StackElement*));
	stack->elements[stack->size - 1] = element;
}

void rsRemoveScope(RuntimeStack *stack, char *functionName){
	for(int i = stack->size - 1; i >= 0; i--){
		if(strcmp(stack->elements[i]->functionName, functionName) == 0) continue;
		if(i == stack->size - 1) return;

		for(int k = i + 1; k < stack->size; k++){
			free(stack->elements[k]);
			stack->elements[k] = NULL;
		}

		stack->size = i + 1;
		return;
	}

	free(stack->elements);
	stack->size = 0;
	stack->elements = NULL;
}

StackElement *rsFind(RuntimeStack *stack, char *name){
	for(int i = stack->size - 1; i >= 0; i--){
		if(strcmp(stack->elements[i]->name, name) == 0) 
			return stack->elements[i];
	}

	return NULL;
}

FunctionToken *rsGetFunction(RuntimeStack *stack, char *name){
	for(int i = 0; i < stack->fileToken->size; i++){
		if(strcmp(stack->fileToken->functions[i]->name, name) == 0){
			return stack->fileToken->functions[i];
		}
	}

	return NULL;
}
