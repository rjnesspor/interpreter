#ifndef VARIABLE_H
#define VARIABLE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_VARIABLES 20

typedef union {
    int intValue;
    char stringValue[128];
} Value;

// 0: int, 1: string
typedef struct {
    char name[64];
    int type;
    Value value;
} Variable;

typedef struct {
    Variable* variables;
    int count;
} VariableTable;

Variable* getVariable(char* name);
void addVariable(Variable var);
void printVariable(Variable* var);
void printVariables();
void process(char* line, int lineNumber, bool* executionEnabled);
int parseIntegerMath(char* arg1, char* arg2, char operator, int lineNumber);
#endif