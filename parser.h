#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"
#include "ast.h"


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
Token* advance();
void free_ast(ASTNode* node);
Token* currentToken();
int match(TokenType type, const char* text);
void expect(TokenType type, const char* text);
char* expectText(TokenType type);
const char* tokenTypeName(TokenType type);
ASTNode* createNode(ASTNodeType type);
void addChild(ASTNode* parent, ASTNode* child);
void putError();

#endif