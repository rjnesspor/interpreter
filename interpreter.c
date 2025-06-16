#include "interpreter.h"
#include "ast.h"

Variable symbolTable[MAX_VARIABLES];
int vars = 0;

Value* getVariable(const char* name) {
    for (int i = 0; i < vars; i++) {
        if (strcmp(symbolTable[i].name, name) == 0) {
            return &symbolTable[i].value;
        }
    }
    return NULL;
}

void setVariable(const char* name, Value val) {
    Value* existing = getVariable(name);
    if (existing) {
        *existing = val;
    } else {
        strcpy(symbolTable[vars].name, name);
        symbolTable[vars++].value = val;
    }
}

Value eval(ASTNode* node) {
    switch(node->type) {

        case AST_LITERAL:

            Value val;
            if (strncmp(node->varType, "integer", 7) == 0) {
                val.type = TYPE_INT;
                val.intValue = atoi(node->value);
            } else if (strncmp(node->varType, "string", 6) == 0) {
                val.type = TYPE_STRING;
                strcpy(val.strValue, node->value);
            }

            return val;
        case AST_VARIABLE:
            Value* v = getVariable(node->name);
            if (!v) {
                fprintf(stderr, "Runtime error: variable '%s' is undefined\n", node->name);
                exit(1);
            }
            return *v;
        case AST_BINOP:
            Value left = eval(node->left);
            Value right = eval(node->right);
            if (left.type != TYPE_INT || right.type != TYPE_INT) {
                fprintf(stderr, "Runtime error: binary operation '%s' is undefined for type string\n", node->value);
                exit(1);
            }
            Value result;
            result.type = TYPE_INT;
            switch (node->value[0]) {
                case '+':
                    result.intValue = left.intValue + right.intValue;
                    break;
                case '-':
                    result.intValue = left.intValue - right.intValue;
                    break;
                case '*':
                    result.intValue = left.intValue * right.intValue;
                    break;
                case '/':
                    result.intValue = left.intValue / right.intValue;
                    break;
                case '=':
                    result.intValue = left.intValue == right.intValue;
                    break;
                case '>':
                    result.intValue = left.intValue > right.intValue;
                    break;
                case '<':
                    result.intValue = left.intValue < right.intValue;
                    break;
                default:
                    fprintf(stderr, "Runtime error: Unrecognized binary operator '%c'.\n", node->value[0]);
                    exit(1);
            }
            return result;
        default:
            printf("oops %s", node->name);
            exit(1);
    }
}

void execStatement(ASTNode* node) {
    switch (node->type) {

        case AST_DEFINE:
        case AST_REDEFINE:
            Value val = eval(node->right);
            setVariable(node->name, val);
            break;
        case AST_PRINT:
            Value printVal = eval(node->right);
            if (printVal.type == TYPE_INT) {
                printf("%d\n", printVal.intValue);
            } else {
                printf("%s\n", printVal.strValue);
            }
            break;
        case AST_BLOCK:
            for (int i = 0; i < node->childCount; i++) {
                execStatement(node->children[i]);
            }
            break;
        case AST_INPUT:
            Value inputVal;
            if (strncmp(node->varType, "string", 6) == 0) {
                inputVal.type = TYPE_STRING;
                fscanf(stdin, "%[^\n]", &inputVal.strValue);
                setVariable(node->name, inputVal);
            } else if (strncmp(node->varType, "integer", 7) == 0) {
                inputVal.type = TYPE_INT;
                fscanf(stdin, "%d", &inputVal.intValue);
                setVariable(node->name, inputVal);
            }
            break;
        case AST_IF:
            Value ifResult = eval(node->condition);
            if (ifResult.intValue) {
                execStatement(node->children[0]);
            }
            break;
        case AST_EOL:
            // program finished
            break;
        case AST_LEAVE:
            exit(atoi(node->value));
        default:
            printf("Runtime error: Unrecognized node type %s\n", nodename(node->type));

            exit(1);

    }
}

void interpret(ASTNode* root) {
    if (!root || root->type != AST_BLOCK) {
        fprintf(stderr, "program root must be BLOCK\n");
        exit(1);
    }
    execStatement(root);
}

const char *nodename(ASTNodeType t) {
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
        default:            return "??";
    }
}