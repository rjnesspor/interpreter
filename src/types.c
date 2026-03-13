#include "types.h"

#include <stdlib.h>
#include <string.h>

static TypeDesc _int = {TYPE_INT, NULL};
static TypeDesc _string = {TYPE_STRING, NULL};
static TypeDesc _float = {TYPE_FLOAT, NULL};
static TypeDesc _void = {TYPE_VOID, NULL};
static TypeDesc _function = {TYPE_FUNCTION, NULL};

TypeDesc* typeInt() { 
    return &_int;
}

TypeDesc* typeString() {
    return &_string;
}

TypeDesc* typeFloat() {
    return &_float;
}

TypeDesc* typeVoid() {
    return &_void;
}

TypeDesc* typeFunction() {
    return &_function;
}

TypeDesc* typeArray(TypeDesc* elementType) {
    TypeDesc* t = malloc(sizeof(TypeDesc));
    t->base = TYPE_ARRAY;
    t->elementType = elementType;
    return t;
}

const char* typeName(TypeDesc* t) {
    switch (t->base) {
        case TYPE_INT: return "integer";
        case TYPE_FLOAT: return "float";
        case TYPE_STRING: return "string";
        case TYPE_ARRAY: return "array";
        case TYPE_VOID: return "void";
        case TYPE_FUNCTION: return "function";
        default: return "unknown";
    }
}

int typeEquals(TypeDesc* a, TypeDesc* b) {
    if (a->base != b->base) return 0;
    if (a->base == TYPE_ARRAY) return typeEquals(a->elementType, b->elementType);
    return 1;
}