#ifndef INTERP_H
#define INTERP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "ast.h"
#include "types.h"
#include "utils.h"

#define MAX_VARIABLES 128
#define MAX_SCOPES 32
#define GLOBAL_SCOPE_INDEX 0
#define MAX_FUNCTIONS 32
#define MAX_PARAMS 8

typedef struct {
    TypeDesc* typeDesc;
    union {
        int intValue;
        float floatValue;
        char strValue[256];
        struct {
            struct Value* elements;
            int length;
        } arrayValue;
        ASTNode* function;
    };
} Value;

typedef struct {
    char name[64];
    Value value;
} Variable;

typedef struct {
    Variable vars[MAX_VARIABLES];
    int count;
} Scope;

typedef struct {
    char name[64];
    ASTNode* body;
} Function;

typedef struct {
    ASTNode* function;
    Value returnValue;
    int hasReturned;
} CallFrame;

ASTNode* getFunction(const char* name);
void defineFunction(ASTNode* node);

void pushScope();
void popScope();
Value* lookupVariable(const char* name);
void defineVariable(const char* name, TypeDesc* declaredType, Value val);
void setVariable(const char* name, Value val);

Value* getVariable(const char* name);
void setVariable(const char* name, Value val);
void interpret(ASTNode* root);
const char *nodename(ASTNodeType t);
bool isInt(const char* str);

#endif