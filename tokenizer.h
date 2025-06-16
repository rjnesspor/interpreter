#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_TOKENS 128

typedef enum {
    TOK_KEYWORD,
    TOK_IDENTIFIER,
    TOK_NUMBER,
    TOK_STRING,
    TOK_PLUS,
    TOK_MINUS,
    TOK_MUL,
    TOK_DIV,
    TOK_EQ,
    TOK_GT,
    TOK_LT,
    TOK_EOL,
    TOK_EOF
} TokenType;

typedef struct {
    TokenType type;
    char value[256];
} Token;

int tokenize(const char* input, Token tokens[], int cap);
int isKeyword(const char* str);

#endif