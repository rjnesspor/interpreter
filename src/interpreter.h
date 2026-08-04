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

typedef struct Value {
    TypeDesc* typeDesc;
    union {
        int intValue;
        float floatValue;
        char strValue[512];
        struct {
            struct Value* elements;
            int length;
        } arrayValue;
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
    int scopeBase;
} CallFrame;

typedef Value (*BinopHandler)(Value left, Value right);

typedef struct {
    BaseType left;
    BaseType right;
    char op[3];
    BinopHandler handler;
} BinopRule;

Value int_int_add(Value left, Value right);
Value int_int_sub(Value left, Value right);
Value int_int_mul(Value left, Value right);
Value int_int_div(Value left, Value right);
Value int_int_mod(Value left, Value right);
Value int_int_eq(Value left, Value right);
Value int_int_lt(Value left, Value right);
Value int_int_gt(Value left, Value right);
Value int_int_neq(Value left, Value right);
Value int_int_lte(Value left, Value right);
Value int_int_gte(Value left, Value right);
Value float_float_add(Value left, Value right);
Value float_float_sub(Value left, Value right);
Value float_float_mul(Value left, Value right);
Value float_float_div(Value left, Value right);
Value float_float_eq(Value left, Value right);
Value float_float_lt(Value left, Value right);
Value float_float_gt(Value left, Value right);
Value float_float_neq(Value left, Value right);
Value float_float_lte(Value left, Value right);
Value float_float_gte(Value left, Value right);
Value string_string_add(Value left, Value right);
Value string_string_eq(Value left, Value right);
Value string_string_neq(Value left, Value right);
Value string_string_lt(Value left, Value right);
Value string_string_gt(Value left, Value right);

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