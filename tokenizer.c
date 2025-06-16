#include "tokenizer.h"
#include "utils.h"

int tokenize(const char* input, Token tokens[], int cap) {

    int i = 0; // char index
    int n = 0; // token count
    char buff[1024]; // mutable copy of input
    strncpy(buff, input, sizeof(buff));
    int lineNum = 1;

    while (buff[i]) {

        if (buff[i] == '\n') {
            tokens[n].type = TOK_EOL;
            tokens[n].value[0] = '\0';
            n++;
            i++;
            lineNum++;
            continue;
        }

        if (isspace(buff[i])) {
            i++;
            continue;
        }

        // string literals
        if (buff[i] == '"') {
            i++;
            int start = i;
            while (buff[i] && buff[i] != '"' && buff[i]  != '\n') {
                i++;
            }

            if (buff[i] != '"') {
                putError(lineNum);
                fprintf(stderr, "Unterminated string literal.\n");
                return -1;
            }

            int len = i - start;
            if (n >= cap) return -1;
            tokens[n].type = TOK_STRING;
            strncpy(tokens[n].value, &buff[start], len);
            tokens[n].value[len] = '\0';
            n++;
            i++;
            continue;
        }

        // number literals
        if (isdigit(buff[i])) {
            int start = i;
            while (isdigit(buff[i])) {
                i++;
            }
            int len = i - start;
            if (n >= cap) return -1;
            tokens[n].type = TOK_NUMBER;
            strncpy(tokens[n].value, &buff[start], len);
            tokens[n].value[len] = '\0';
            n++;
            continue;
        }

        // identifiers/keywords
        if (isalpha(buff[i])) {
            int start = i;
            while (isalnum(buff[i]) || buff[i] == '_') {
                i++;
            }
            int len = i - start;
            if (n >= cap) return -1;
            strncpy(tokens[n].value, &buff[start], len);
            tokens[n].value[len] = '\0';
            tokens[n].type = isKeyword(tokens[n].value) ? TOK_KEYWORD : TOK_IDENTIFIER;
            n++;
            continue;
        }

        // operators
        if (strchr("+-*/><=", buff[i])) {
            if (n >= cap) return -1;
            if (buff[i] == '+') tokens[n].type = TOK_PLUS;
            if (buff[i] == '-') tokens[n].type = TOK_MINUS;
            if (buff[i] == '*') tokens[n].type = TOK_MUL;
            if (buff[i] == '/') tokens[n].type = TOK_DIV;
            if (buff[i] == '>') tokens[n].type = TOK_GT;
            if (buff[i] == '<') tokens[n].type = TOK_LT;
            if (buff[i] == '=') tokens[n].type = TOK_EQ;
            tokens[n].value[0] = buff[i];
            tokens[n].value[1] = '\0';
            n++;
            i++;
            continue;
        }

        putError(lineNum);
        fprintf(stderr, "Unexpected character '%c'.\n", buff[i]);
        return -1;

    }

    if (n < cap) {
        tokens[n].type = TOK_EOL;
        tokens[n].value[0] = '\0';
        n++;
    }
    
    return n;

}

int isKeyword(const char* str) {
    static const char *kw[] = {
        "define", "redefine", "print", "input", "if", "endif", "leave",
        "loop", "endloop", "integer", "string", "as", NULL
    };

    for (int i = 0; kw[i]; ++i) {
        if (strcmp(kw[i], str) == 0) {
            return 1;
        }
    }
    return 0;
}