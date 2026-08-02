#ifndef TYPES_H
#define TYPES_H

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_LIST,
    TYPE_FUNCTION,
    TYPE_VOID
} BaseType;

typedef struct TypeDesc {
    BaseType base;
    struct TypeDesc* elementType;
} TypeDesc;

TypeDesc* typeInt();
TypeDesc* typeString();
TypeDesc* typeFloat();
TypeDesc* typeFunction();
TypeDesc* typeVoid();
TypeDesc* typeList(TypeDesc* elementType);

const char* typeName(TypeDesc* t);
int typeEquals(TypeDesc* a, TypeDesc* b);

#endif