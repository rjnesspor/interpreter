#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"
#include "ast.h"

#define LOOP_END_KEYWORD "endloop"
#define IF_END_KEYWORD "endif"
#define FUNC_END_KEYWORD "endf"

ASTNode* parseProgram(Token* tokens, int tokenCount);
ASTNode* parseStatement();
ASTNode* parseDefine();
ASTNode* parseRedefine();
ASTNode* parseExpression();
ASTNode* parsePrint();
ASTNode* parseInput();
ASTNode* parseBlock(const char* endKeyword);
ASTNode* parseIf();
ASTNode* parseLoop();
ASTNode* parseLeave();
ASTNode* parseCall();
Token* advance();
void free_ast(ASTNode* node);
Token* currentToken();
int match(TokenType type, const char* text);
void expect(TokenType type, const char* text);
char* expectText(TokenType type);
const char* tokenTypeName(TokenType type);
ASTNode* createNode(ASTNodeType type);
void addChild(ASTNode* parent, ASTNode* child);
void addParam(ASTNode* func, ASTNode* param);
void addArg(ASTNode* func, ASTNode* arg);

#endif