#include "tokenizer.h"
#include "parser.h"
#include "interpreter.h"

void print_ast(const ASTNode *node, int depth);
const char *node_type_name(ASTNodeType t);
void indent(int n);
void free_ast(ASTNode* node);
void free_program();

#define MAX_SOURCE 8192

ASTNode* program;

int main(int argc, char const *argv[]) {
    
    FILE* file = fopen(argv[1], "r");  
    
    char source[MAX_SOURCE] = "";
    char line[256];

    while (fgets(line, sizeof(line), file)) {
        strcat(source, line);
    }

    fclose(file);

    Token tokens[1024];
    int tokenCount = tokenize(source, tokens, 1024);

    program = parseProgram(tokens, tokenCount);
    //for (int i = 0; i < tokenCount; i++) {
        //printf("%s %s\n", tokenTypeName(tokens[i].type), tokens[i].value);
    //}
    //print_ast(program, 0);

    atexit(free_program);
    interpret(program);

    return 0;
}

void free_program() {
    free_ast(program);
}

void free_ast(ASTNode* node) {
    if (!node) return;

    if (node->left) {
        free_ast(node->left);
    }

    if (node->right) {
        free_ast(node->right);
    }

    if (node->condition) {
        free_ast(node->condition);
    }

    if (node->children) {
        for (int i = 0; i < node->childCount; i++) {
            free_ast(node->children[i]);
        }
        free(node->children);
    }

    free(node);
}

void print_ast(const ASTNode *node, int depth) {
    if (node == NULL) {
        indent(depth); 
        puts("(null)");
        return;
    }

    indent(depth);
    printf("%s ", node_type_name(node->type));

    switch (node->type) {
        case AST_DEFINE:
        case AST_REDEFINE:
            printf(" %s %s %s", node->varType, node->name, node->value);
            break;

        case AST_FUNCTION:
        case AST_VARIABLE:
            printf("%s", node->name);
            break;

        case AST_CALL:
        case AST_LITERAL:
            printf(" \"%s\"", node->value);
            break;

        case AST_BINOP:
            printf(" %s", node->value);
            break;

        case AST_PRINT:
            printf(" %s", node->value);
            break;

        case AST_INPUT:
            printf(" %s %s", node->varType, node->name);
            break;

        default:
            break;
    }
    putchar('\n');


    if (node->condition) {
        indent(depth + 2);
        puts("condition:");
        print_ast(node->condition, depth + 4);
    }

    if (node->left) {
        indent(depth + 2);
        puts("left:");
        print_ast(node->left, depth + 4);
    }
    if (node->right) {
        indent(depth + 2);
        puts("right:");
        print_ast(node->right, depth + 4);
    }

    if (node->childCount > 0 && node->children) {
        for (int i = 0; i < node->childCount; ++i) {
            indent(depth + 2);
            printf("child[%d]:\n", i);
            print_ast(node->children[i], depth + 4);
        }
    }
}

const char *node_type_name(ASTNodeType t) {
    switch (t) {
        case AST_EOL:       return "EOL";
        case AST_DEFINE:    return "DEFINE";
        case AST_REDEFINE:  return "REDEFINE";
        case AST_PRINT:     return "PRINT";
        case AST_INPUT:     return "INPUT";
        case AST_IF:        return "IF";
        case AST_LOOP:      return "LOOP";
        case AST_LEAVE:     return "LEAVE";
        case AST_BLOCK:     return "BLOCK";
        case AST_LITERAL:   return "LITERAL";
        case AST_VARIABLE:  return "VARIABLE";
        case AST_BINOP:     return "BINOP";
        case AST_FUNCTION:  return "FUNCTION";
        case AST_CALL:       return "FCALL";
        default:            return "??";
    }
}

void indent(int n) {
    while (n-- > 0) putchar(' ');
}