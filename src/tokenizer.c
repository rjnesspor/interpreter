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

        // comments, skip them
        if (buff[i] == '?') {
            i++;
            while (buff[i] && buff[i] != '\n') {
                i++;
            }
            continue;
        }

        // string literals
        if (buff[i] == '"') {
            i++;
            int start = i;
            while (buff[i] && buff[i] != '"' && buff[i] != '\n') {
                i++;
            }

            if (buff[i] != '"') {
                putError(lineNum);
                fprintf(stderr, "Unterminated string literal.\n");
                exit(1);
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
            int decimal = 0;
            while (isdigit(buff[i]) || (buff[i] == '.' && !decimal)) {
                if (buff[i] == '.') {
                    if (!isdigit(buff[i + 1])) {
                        putError(lineNum);
                        fprintf(stderr, "Expected a number after '.'.\n");
                        exit(1);
                    }
                    decimal = 1;
                }
                i++;
            }
            int len = i - start;
            if (n >= cap) return -1;
            tokens[n].type = decimal ? TOK_FLOAT : TOK_INTEGER;
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
        if (strchr("+-*/><=(),[]%!", buff[i])) {
            if (n >= cap) return -1;
            int start = i; // start of lexeme
            int len = 1;
            if (buff[i] == '+') tokens[n].type = TOK_PLUS;
            else if (buff[i] == '-') tokens[n].type = TOK_MINUS;
            else if (buff[i] == '*') tokens[n].type = TOK_MUL;
            else if (buff[i] == '/') tokens[n].type = TOK_DIV;
            else if (buff[i] == '%') tokens[n].type = TOK_MOD;
            else if (buff[i] == '>') { // peek ahead to see if it is any of our 2 character ops
                if (buff[i + 1] && buff[i + 1] == '=') {
                    tokens[n].type = TOK_GTE;
                    len = 2;
                } else {
                    tokens[n].type = TOK_GT;
                }
            }
            else if (buff[i] == '<') {
                if (buff[i + 1] && buff[i + 1] == '=') {
                    tokens[n].type = TOK_LTE;
                    len = 2;
                } else {
                    tokens[n].type = TOK_LT;
                }
            }
            else if (buff[i] == '!') {
                if (buff[i + 1] && buff[i + 1] == '=') {
                    tokens[n].type = TOK_NEQ;
                    len = 2;
                } else {
                    putError(lineNum);
                    fprintf(stderr, "Unexpected character '%c'.\n", buff[i]);
                    exit(1);
                }
            }
            else if (buff[i] == '=') tokens[n].type = TOK_EQ;
            else if (buff[i] == '(') tokens[n].type = TOK_LPAREN;
            else if (buff[i] == ')') tokens[n].type = TOK_RPAREN;
            else if (buff[i] == ',') tokens[n].type = TOK_COMMA;
            else if (buff[i] == '[') tokens[n].type = TOK_LBRACKET;
            else if (buff[i] == ']') tokens[n].type = TOK_RBRACKET;
            strncpy(tokens[n].value, &buff[start], len);
            tokens[n].value[len] = '\0';
            n++;
            i += len;
            continue;
        }

        putError(lineNum);
        fprintf(stderr, "Unexpected character '%c'.\n", buff[i]);
        exit(1);

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
        "loop", "endl", "integer", "string", "as", "func", "endf", 
        "call", "with", "float", "list", "of", NULL
    };

    for (int i = 0; kw[i]; ++i) {
        if (strcmp(kw[i], str) == 0) {
            return 1;
        }
    }
    return 0;
}