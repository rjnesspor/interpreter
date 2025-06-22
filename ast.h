#ifndef AST_H
#define AST_H

typedef enum {
    AST_EOL,
    AST_EOF,
    AST_DEFINE,
    AST_REDEFINE,
    AST_PRINT,
    AST_INPUT,
    AST_IF,
    AST_LOOP,
    AST_LEAVE,
    AST_BLOCK,
    AST_LITERAL,
    AST_VARIABLE,
    AST_BINOP,
    AST_FUNCTION,
    AST_CALL
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;

    char name[64];
    char varType[16];

    char value[128];

    struct ASTNode* left;
    struct ASTNode* right;

    struct ASTNode** children;
    int childCount;

    struct ASTNode* condition;

    int lineNum;

    struct ASTNode** parameters;
    int paramCount;

    struct ASTNode** args;
    int argCount;
} ASTNode;

ASTNode* createNode(ASTNodeType type);

#endif