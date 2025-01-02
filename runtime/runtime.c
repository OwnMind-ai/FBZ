#include "runtime.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void start(FileToken *file){
	RuntimeStack *stack = rsCreate(file);

	FunctionToken *mainToken = NULL;
	for(int i = 0; i < file->size; i++)
		if(strcmp(file->functions[i]->name, "main") == 0)
			mainToken = file->functions[i];

	if(mainToken == NULL)
		stopRuntime("No main function found");

	executeFunction(mainToken, stack);
}

double computeOperation(OperationToken *call, RuntimeStack *stack, char *functionName);

double computeNumber(Token *token, RuntimeStack *stack, char *functionName){
	StackElement *element;
	switch (token->tokenType){
		case NUMBER:
			return token->number;
		case VARIABLE:
			element = rsFind(stack, token->keyword);
			if(element == NULL)
				stopRuntime("No variable named '%s'", token->keyword);

			if(element->type != NUMBER_TYPE)
				stopRuntime("Unable to convert variable '%s' to a number", element->name);

			return *((double*) element->value);
		case OPERATION: 
			return computeOperation(token->operation, stack, functionName);
		default: stopRuntime("Unable to convert to a number");
	}

	return 0;
}

double computeOperation(OperationToken *operation, RuntimeStack *stack, char *functionName){
	if(operation->op[0] == '='){
		if(operation->left->tokenType != VARIABLE) stopRuntime("Unable to apply '=' operator");

		char *variable = operation->left->variable;
		StackElement *element = rsFind(stack, variable);
		double right = computeNumber(operation->right, stack, functionName);

		if(element->type != NUMBER_TYPE) stopRuntime("Invalid type");
		*((double*) element->value) = right;
		return right;
	}

	double left = computeNumber(operation->left, stack, functionName);
	double right = computeNumber(operation->right, stack, functionName);

	switch(operation->op[0]){
		case '+': return left + right;
		case '-': return left - right;
		case '/': return left / right;
		case '*': return left * right;
		case '%': return fmod(left, right);
		default: stopRuntime("Invalid operator: %s", operation->op);
	}
	return 0;
}

void computeCall(CallToken *call, RuntimeStack *stack, char *functionName){
	FunctionToken *function = rsGetFunction(stack, call->function);
	if(function == NULL) stopRuntime("No function named '%s'", call->function);

	if(call->argsLen != function->parametersLength - 1)
		stopRuntime("Unable to call function '%s': expected %d parameters but found %d",
			  function->name, function->parametersLength - 1, call->argsLen);

	for(int i = 0; i < call->argsLen; i++){
		Token *arg = call->args[i];
		void *value;
		StackElementType type;

		if(arg->tokenType == STRING) {
			value = (void*) arg->string;
			type = STRING_TYPE;
		} else {
			double *pointer = malloc(sizeof(double));

			*pointer = computeNumber(arg, stack, functionName);
			value = pointer;
			type = NUMBER_TYPE;
		}

		rsAdd(stack, rsCreateElement(function->name, function->parameters[i + 1], type, value));
	}

	executeFunction(function, stack);
}

void printResultingToken(Token *token, RuntimeStack *stack, char *functionName){
	StackElement *element;
	switch (token->tokenType) {
		case CALL:
			computeCall(token->call, stack, functionName);
			break;
		case STRING:
			printf("%s", token->string);
			break;
		case VARIABLE:
			element = rsFind(stack, token->variable);
			if(element == NULL)
				stopRuntime("No variable named '%s'", element->name);

			if(element->type == STRING_TYPE){
				printf("%s", (char*) element->value);
			} else if(element->type == NUMBER_TYPE) {
				printf("%f", *((double*) element->value));
			} else 
			stopRuntime("Unvalid variable type");
			break;
		case NUMBER:
			printf("%f", token->number);
			break;
		case OPERATION:
			printf("%f", computeOperation(token->operation, stack, functionName));
			break;
		default: stopRuntime("Unable to print");
	}
}

void executeFunction(FunctionToken *function, RuntimeStack *stack){
	if(function->parametersLength == 0) 
		stopRuntime("No counter detected at function '%s'", function->name);

	char *counterName = function->parameters[0];
	double *counterValue = malloc(sizeof(double));
	*counterValue = computeNumber(function->rangeToken->from, stack, function->name);
	StackElement *counter = rsCreateElement(function->name, counterName, NUMBER_TYPE, counterValue);
	rsAdd(stack, counter);

	while(*counterValue <= computeNumber(function->rangeToken->to, stack, function->name)){
		Token *elseCondition = NULL;
		int conditionFound = 0;
		for(int i = 0; i < function->conditionsLength; i++){
			Token *condition = function->conditions[i];
			if(condition->tokenType != OPERATION && condition->operation->op[1] != '>')
				stopRuntime("Invalid condition");

			Token *left = condition->operation->left;
			if(left->tokenType == KEYWORD && strcmp(left->keyword, "else") == 0){
				if(elseCondition != NULL) stopRuntime("Duplicate else case in function '%s'", function->name);

				elseCondition = condition;
				continue;
			}

			double number = computeNumber(left, stack, function->name);
			if(number != 0 && fmod(*counterValue, number) == 0){
				conditionFound = 1;
				printResultingToken(condition->operation->right, stack, function->name);
			}
		}

		if(conditionFound) printf("\n");
		else if(elseCondition != NULL){
			printResultingToken(elseCondition->operation->right, stack, function->name);
			printf("\n");
		}

		*counterValue += computeNumber(function->rangeToken->step, stack, function->name);
	}

	rsRemoveScope(stack, function->name);
}

void stopRuntime(char *format, ...){
	va_list args;
	va_start(args, format);
	char out[100];
	vsnprintf(out, 100, format, args);
	va_end(args);

	fprintf(stderr, "RuntimeError:\n%s\n", out);
	exit(1);
}
