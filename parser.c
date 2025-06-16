#include "parser.h"
#include "utils.h"

Token* tokens;
int tokenCount;
int pos;

int lineNum;

ASTNode* parseProgram(Token* t, int count) {
    pos = 0;
    tokens = t;
    tokenCount = count;
    lineNum = 1;
    
    ASTNode* program = createNode(AST_BLOCK);

    while (pos < tokenCount) {
        ASTNode* stmt = parseStatement();
        addChild(program, stmt);
    }

    return program;
}

ASTNode* parseStatement() {
    Token* t = currentToken();

    if (t->type == TOK_EOL) {
        ASTNode* node = createNode(AST_EOL);
        lineNum++;
        pos++;
        return node;
    }

    if (t->type != TOK_KEYWORD) {
        putError(lineNum);
        fprintf(stderr, "Expected a keyword, instead got '%s'.\n", t->value);
        exit(1);
    }

    if (strcmp(t->value, "define") == 0) return parseDefine();
    if (strcmp(t->value, "redefine") == 0) return parseRedefine();
    if (strcmp(t->value, "print") == 0) return parsePrint();
    if (strcmp(t->value, "input") == 0) return parseInput();
    if (strcmp(t->value, "if") == 0) return parseIf();
    if (strcmp(t->value, "loop") == 0) return parseLoop();
    if (strcmp(t->value, "leave") == 0) return parseLeave();

    putError(lineNum);
    fprintf(stderr, "Unknown keyword '%s'.\n", t->value);
    exit(1);
}

ASTNode* parseDefine() {
    expect(TOK_KEYWORD, "define");

    char* type = expectText(TOK_KEYWORD);
    char* name = expectText(TOK_IDENTIFIER);

    expect(TOK_KEYWORD, "as");

    ASTNode* node = createNode(AST_DEFINE);
    strcpy(node->varType, type);
    strcpy(node->name, name);

    Token* val = currentToken();
    if (strcmp(type, "string") == 0) {
        if (val->type != TOK_STRING) {
            putError(lineNum);
            fprintf(stderr, "Expected a string literal after 'as'.\n");
            exit(1);
        }
        ASTNode* n = createNode(AST_LITERAL);
        strcpy(n->value, val->value);
        strcpy(n->varType, "string");
        node->right = n;
    } else if (strcmp(type, "integer") == 0) {
        if (val->type != TOK_NUMBER) {
            putError(lineNum);
            fprintf(stderr, "Expected a number literal after 'as'.\n");
            exit(1);
        }
        ASTNode* n = createNode(AST_LITERAL);
        strcpy(n->value, val->value);
        strcpy(n->varType, "integer");
        node->right = n;
    }

    pos++;

    return node;
}

ASTNode* parseRedefine() {
    expect(TOK_KEYWORD, "redefine");

    char* name = expectText(TOK_IDENTIFIER);
    expect(TOK_KEYWORD, "as");

    if (match(TOK_PLUS, "+") || match(TOK_MINUS, "-") || match(TOK_MUL, "*") || match(TOK_DIV, "/")) {
        putError(lineNum);
        fprintf(stderr, "Unexpected operator at start of expression.\n");
        exit(1);
    }

    ASTNode* node = createNode(AST_REDEFINE);
    strcpy(node->name, name);

    Token* left = advance();
    Token* t = currentToken();

    if (t->type == TOK_PLUS || t->type == TOK_MINUS || t->type == TOK_MUL || t->type == TOK_DIV) {
        // binary op
        ASTNode* binop = createNode(AST_BINOP);
        strcpy(binop->value, t->value);
        advance();

        Token* right = advance();

        // left 
        if (left->type == TOK_NUMBER ||  left->type == TOK_STRING) {
            binop->left = createNode(AST_LITERAL);
            strcpy(binop->left->value, left->value);
            if (left->type == TOK_NUMBER) {
                strcpy(binop->left->varType, "integer");
            } else if (left->type == TOK_STRING) {
                strcpy(binop->left->varType, "string");
            }
        } else if (left->type == TOK_IDENTIFIER) {
            binop->left = createNode(AST_VARIABLE);
            strcpy(binop->left->name, left->value);
        } else {
            putError(lineNum);
            fprintf(stderr, "Unexpected token '%s' as left operand of expression.\n", left->value);
            exit(1);
        }

        // right
        if (right->type == TOK_NUMBER ||  right->type == TOK_STRING) {
            binop->right = createNode(AST_LITERAL);
            strcpy(binop->right->value, right->value);
            if (right->type == TOK_NUMBER) {
                strcpy(binop->right->varType, "integer");
            } else if (right->type == TOK_STRING) {
                strcpy(binop->right->varType, "string");
            }
        } else if (right->type == TOK_IDENTIFIER) {
            binop->right = createNode(AST_VARIABLE);
            strcpy(binop->right->name, right->value);
        } else {
            putError(lineNum);
            fprintf(stderr, "Unexpected token '%s' as right operand of expression.\n", right->value);
            exit(1);
        }

        node->right = binop;

    } else {
        // simple value
        if (left->type == TOK_NUMBER || left->type == TOK_STRING) {
            ASTNode* val = createNode(AST_LITERAL);
            strcpy(val->value, left->value);
            node->right = val;
        } else if (left->type == TOK_IDENTIFIER) {
            ASTNode* var = createNode(AST_VARIABLE);
            strcpy(var->name, left->value);
            node->right = var;
        } else {
            putError(lineNum);
            fprintf(stderr, "Unexpected token '%s' after 'as' in expression.\n", left->value);
            exit(1);
        }
    }

    return node;
}

ASTNode* parsePrint() {
    expect(TOK_KEYWORD, "print");

    ASTNode* node = createNode(AST_PRINT);

    Token* t = currentToken();
    if (t->type != TOK_IDENTIFIER) {
        putError(lineNum);
        fprintf(stderr, "Expected an identifier to print.\n");
        exit(1);
    }

    ASTNode* var = createNode(AST_VARIABLE);
    strcpy(var->name, t->value);
    node->right = var;

    pos++;
    return node;
}

ASTNode* parseExpression() { // x < 10, x + 5, y + x, x = 1
    Token* left = advance();

    Token* t = currentToken();
    ASTNode* expr = NULL;
    
    if (t->type == TOK_PLUS || t->type == TOK_MINUS || t->type == TOK_MUL || t->type == TOK_DIV || t->type == TOK_GT || t->type == TOK_LT || t->type == TOK_EQ) {
        advance();
        expr = createNode(AST_BINOP);
        strcpy(expr->value, t->value);
        Token* right = advance();

        if (left->type == TOK_IDENTIFIER) {
            expr->left = createNode(AST_VARIABLE);
            strcpy(expr->left->name, left->value);
        } else if (left->type == TOK_NUMBER) {
            expr->left = createNode(AST_LITERAL);
            strcpy(expr->left->value, left->value);
            strcpy(expr->left->varType, "integer");
        } else {
            putError(lineNum);
            fprintf(stderr, "Unexpected token '%s' as left operand of expression.\n", left->value);
            exit(1);
        }

        if (right->type == TOK_IDENTIFIER) {
            expr->right = createNode(AST_VARIABLE);
            strcpy(expr->right->name, right->value);
        } else if (right->type == TOK_NUMBER) {
            expr->right = createNode(AST_LITERAL);
            strcpy(expr->right->value, right->value);
            strcpy(expr->right->varType, "integer");
        } else {
            putError(lineNum);
            fprintf(stderr, "Unexpected token '%s' as right operand of expression.\n", right->value);
            exit(1);
        }
    }

    return expr;
}

ASTNode* parseInput() {
    expect(TOK_KEYWORD, "input");

    char* type = expectText(TOK_KEYWORD);
    expect(TOK_KEYWORD, "as");
    char* name = expectText(TOK_IDENTIFIER);

    ASTNode* node = createNode(AST_INPUT);
    strcpy(node->name, name);
    strcpy(node->varType, type);

    return node;
}

ASTNode* parseBlock(const char* endKeyword) {
    ASTNode* block = createNode(AST_BLOCK);

    while (pos < tokenCount && !(match(TOK_KEYWORD, endKeyword))) {
        ASTNode* stmt = parseStatement();
        addChild(block, stmt);
    }

    expect(TOK_KEYWORD, endKeyword);
    return block;
}

ASTNode* parseIf() {
    expect(TOK_KEYWORD, "if");
    
    ASTNode* cond = parseExpression();

    ASTNode* node = createNode(AST_IF);
    node->condition = cond;
    addChild(node, parseBlock("endif"));

    return node;
}

ASTNode* parseLoop() {
    expect(TOK_KEYWORD, "loop");

    char* value = expectText(TOK_NUMBER);

    ASTNode* node = createNode(AST_LOOP);
    strcpy(node->value, value);
    addChild(node, parseBlock("endloop"));

    return node;
}

ASTNode* parseLeave() {
    expect(TOK_KEYWORD, "leave");

    char* status = expectText(TOK_NUMBER);
    ASTNode* node = createNode(AST_LEAVE);
    strcpy(node->value, status);
    
    return node;
}

Token* currentToken() {
    return &tokens[pos];
}

Token* advance() {
    return &tokens[pos++];
}

int match(TokenType type, const char* text) {
    if (pos >= tokenCount) return 0;
    Token* t = &tokens[pos];
    return t->type == type && (!text || strcmp(t->value, text) == 0);
}

void expect(TokenType type, const char* text) {
    if (!match(type, text)) {
        putError(lineNum);
        fprintf(stderr, "Expected %s '%s', but got '%s'.\n", tokenTypeName(type), text ? text : "(any)", currentToken()->value);
        exit(1);
    }
    pos++;
}

char* expectText(TokenType type) {
    if (pos >= tokenCount || tokens[pos].type != type) {
        putError(lineNum);
        fprintf(stderr, "Expected a(n) '%s' but got a(n) '%s'.\n", tokenTypeName(type), tokenTypeName(tokens[pos].type));
        exit(1);
    }
    return tokens[pos++].value;
}

const char* tokenTypeName(TokenType type) {
    switch (type) {
        case TOK_KEYWORD: return "keyword";
        case TOK_IDENTIFIER: return "identifier";
        case TOK_EOF: return "eof";
        case TOK_EOL: return "eol";
        case TOK_NUMBER: return "number";
        case TOK_PLUS: return "plus";
        case TOK_MINUS: return "minus";
        case TOK_MUL: return "mul";
        case TOK_DIV: return "div";
        case TOK_STRING: return "string";
        default: return "unknown";
    }
}

ASTNode* createNode(ASTNodeType type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        exit(1);
    }

    node->type = type;
    node->name[0] = '\0';
    node->value[0] = '\0';
    node->varType[0] = '\0';
    node->left = NULL;
    node->right = NULL;
    node->condition = NULL;
    node->children = NULL;
    node->childCount = 0;

    node->lineNum = lineNum;

    return node;
}

void addChild(ASTNode* parent, ASTNode* child) {
    parent->children = realloc(parent->children, sizeof(ASTNode*) * (parent->childCount + 1));
    parent->children[parent->childCount++] = child;
}