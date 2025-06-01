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
    char stringValue[64];
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
bool isInt(const char* str);
#endif