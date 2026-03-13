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
    if (strcmp(t->value, "call") == 0) return parseCall();

    putError(lineNum);
    fprintf(stderr, "Unknown keyword '%s'.\n", t->value);
    exit(1);
}

ASTNode* parseDefine() {
    expect(TOK_KEYWORD, "define");

    TypeDesc* typeDesc = parseTypeKeyword();
    char* name = expectText(TOK_IDENTIFIER);

    if (typeEquals(typeDesc, typeFunction())) {
        
        expect(TOK_KEYWORD, "with");
        expect(TOK_LPAREN, "(");

        ASTNode* func = createNode(AST_FUNCTION);
        strcpy(func->name, name);
        func->typeDesc = typeDesc;

        while (pos < tokenCount && !(match(TOK_RPAREN, ")"))) {
            TypeDesc* paramTypeDesc = parseTypeKeyword();
            char* paramName = expectText(TOK_IDENTIFIER);

            ASTNode* param = createNode(AST_VARIABLE);
            strcpy(param->name, paramName);
            param->typeDesc = paramTypeDesc;

            addParam(func, param);

            if (match(TOK_COMMA, ",")) advance();
        }

        expect(TOK_RPAREN, ")");

        addChild(func, parseBlock(FUNC_END_KEYWORD));
        return func;
    }

    expect(TOK_KEYWORD, "as");

    ASTNode* defineNode = createNode(AST_DEFINE);
    strcpy(defineNode->name, name);
    defineNode->typeDesc = typeDesc;
    defineNode->right = parseExpression();

    return defineNode;
}

ASTNode* parseRedefine() {
    expect(TOK_KEYWORD, "redefine");
    char* name = expectText(TOK_IDENTIFIER);
    expect(TOK_KEYWORD, "as");

    ASTNode* node = createNode(AST_REDEFINE);
    strcpy(node->name, name);
    node->right = parseExpression();

    return node;
}

ASTNode* parsePrint() {
    expect(TOK_KEYWORD, "print");

    ASTNode* node = createNode(AST_PRINT);
    node->right = parseAtom();

    pos++;
    return node;
}

ASTNode* parseExpression() {
    ASTNode* left = parseAtom();

    // Thanks Claude!
    if (!(currentToken()->type == TOK_PLUS  ||
          currentToken()->type == TOK_MINUS ||
          currentToken()->type == TOK_MUL   ||
          currentToken()->type == TOK_DIV   ||
          currentToken()->type == TOK_GT    ||
          currentToken()->type == TOK_LT    ||
          currentToken()->type == TOK_EQ)) {
        return left;
    }
    //

    ASTNode* binop = createNode(AST_BINOP);
    strcpy(binop->value, currentToken()->value);
    advance();

    binop->left = left;
    binop->right = parseAtom();

    return binop;
}

ASTNode* parseInput() {
    expect(TOK_KEYWORD, "input");
    TypeDesc* typeDesc = parseTypeKeyword();
    expect(TOK_KEYWORD, "as");
    char* name = expectText(TOK_IDENTIFIER);

    ASTNode* inputNode = createNode(AST_INPUT);
    strcpy(inputNode->name, name);
    inputNode->typeDesc = typeDesc;

    return inputNode;
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
    addChild(node, parseBlock(IF_END_KEYWORD));

    return node;
}

ASTNode* parseLoop() {
    expect(TOK_KEYWORD, "loop");

    ASTNode* node = createNode(AST_LOOP);
    ASTNode* loopCount = parseAtom();
    node->right = loopCount;

    addChild(node, parseBlock(LOOP_END_KEYWORD));

    return node;
}

ASTNode* parseCall() {
    expect(TOK_KEYWORD, "call");

    char* funcName = expectText(TOK_IDENTIFIER);
    ASTNode* node = createNode(AST_CALL);
    strcpy(node->value, funcName);

    expect(TOK_KEYWORD, "with");
    expect(TOK_LPAREN, "(");

    while (pos < tokenCount && !(match(TOK_RPAREN, ")"))) {
        
        ASTNode* arg = parseAtom();
        addArg(node, arg);

        if (match(TOK_COMMA, ",")) advance();
    }

    expect(TOK_RPAREN, ")");

    if (match(TOK_KEYWORD, "as")) {
        advance();
        char* var = expectText(TOK_IDENTIFIER);
        strcpy(node->returnVar, var);
    }

    return node;
}

ASTNode* parseLeave() {
    expect(TOK_KEYWORD, "leave");

    ASTNode* node = createNode(AST_LEAVE);
    ASTNode* returnVal = parseAtom();
    node->right = returnVal;
    
    return node;
}

// An expression atom is anything that can be a value.
// So, we are able to handle literals, variables, etc.
ASTNode* parseAtom() {
    Token* curr = currentToken();
    if (curr->type == TOK_NUMBER) {
        ASTNode* node = createNode(AST_LITERAL);
        strcpy(node->value, expectText(TOK_NUMBER));
        node->typeDesc = typeInt();
        return node;
    } else if (curr->type == TOK_IDENTIFIER) {
        ASTNode* node = createNode(AST_VARIABLE);
        strcpy(node->name, expectText(TOK_IDENTIFIER));
        return node;
    } else if (curr->type == TOK_STRING) {
        ASTNode* node = createNode(AST_LITERAL);
        strcpy(node->value, expectText(TOK_STRING));
        node->typeDesc = typeString();
        return node;
    }
    putError(lineNum);
    fprintf(stderr, "Expected a value but got '%s'.\n", curr->value);
    exit(1);
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
        case TOK_EOL: return "eol";
        case TOK_NUMBER: return "number";
        case TOK_PLUS: return "plus";
        case TOK_MINUS: return "minus";
        case TOK_MUL: return "mul";
        case TOK_DIV: return "div";
        case TOK_STRING: return "string";
        case TOK_LPAREN: return "left paren";
        case TOK_RPAREN: return "right paren";
        case TOK_COMMA: return "comma";
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
    node->typeDesc = NULL;
    node->left = NULL;
    node->right = NULL;
    node->condition = NULL;
    node->children = NULL;
    node->childCount = 0;
    node->parameters = NULL;
    node->paramCount = 0;
    node->args = NULL;
    node->argCount = 0;

    node->lineNum = lineNum;

    node->returnVar[0] = '\0';

    return node;
}

void addChild(ASTNode* parent, ASTNode* child) {
    parent->children = realloc(parent->children, sizeof(ASTNode*) * (parent->childCount + 1));
    parent->children[parent->childCount++] = child;
}

void addParam(ASTNode* func, ASTNode* param) {
    func->parameters = realloc(func->parameters, sizeof(ASTNode*) * (func->paramCount + 1));
    func->parameters[func->paramCount++] = param;
}

void addArg(ASTNode* func, ASTNode* arg) {
    func->args = realloc(func->args, sizeof(ASTNode*) * (func->argCount + 1));
    func->args[func->argCount++] = arg;
}

TypeDesc* parseTypeKeyword() {
    if (match(TOK_KEYWORD, "integer")) {
        advance();
        return typeInt();
    }
    if (match(TOK_KEYWORD, "string")) {
        advance();
        return typeString();
    }
    if (match(TOK_KEYWORD, "float")) {
        advance();
        return typeFloat();
    }
    if (match(TOK_KEYWORD, "func")) {
        advance();
        return typeFunction();
    }
    putError(lineNum);
    fprintf(stderr, "Expected a type keyword but got '%s'.\n", currentToken()->value);
    exit(1);
}