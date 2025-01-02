#ifndef RUNTIME_H
#define RUNTIME_H

#include "../parser/parser.h"

struct RuntimeStack;
typedef struct RuntimeStack RuntimeStack;

enum { NUMBER_TYPE, STRING_TYPE } typedef StackElementType;

struct {
	char *functionName;
	char *name;
	StackElementType type;
	void *value;
} typedef StackElement;

RuntimeStack *rsCreate(FileToken *FileToken);
StackElement *rsCreateElement(char *functionName, char *name, StackElementType type, void *value);
void rsAdd(RuntimeStack *stack, StackElement *element);
void rsRemoveScope(RuntimeStack *stack, char *functionName);
StackElement *rsFind(RuntimeStack *stack, char *name);
FunctionToken *rsGetFunction(RuntimeStack *stack, char *name);

void start(FileToken *file);
void stopRuntime(char *format, ...);
void executeFunction(FunctionToken *function, RuntimeStack *stack);

#endif
