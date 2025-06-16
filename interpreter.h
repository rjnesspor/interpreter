#ifndef INTERP_H
#define INTERP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

#define MAX_VARIABLES 128

typedef enum {
    TYPE_INT,
    TYPE_STRING
} ValueType;

typedef struct {
    ValueType type;
    union {
        int intValue;
        char strValue[256];
    };
} Value;

typedef struct {
    char name[64];
    Value value;
} Variable;

Value* getVariable(const char* name);
void setVariable(const char* name, Value val);
void interpret(ASTNode* root);
const char *nodename(ASTNodeType t);

#endif