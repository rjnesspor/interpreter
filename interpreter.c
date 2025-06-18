#include "interpreter.h"
#include "ast.h"
#include "utils.h"

// new
Scope scopes[MAX_SCOPES];
int scopeTop = -1;

Function functions[MAX_FUNCTIONS];
int functionCount = 0;

void defineFunction(ASTNode* node) {
    for (int i = 0; i < functionCount; i++) {
        if (strcmp(functions[i].name, node->name) == 0) {
            putRError(node->lineNum);
            fprintf(stderr, "Function '%s' is already defined.\n", node->name);
            exit(1);
        }
    }
    
    if (functionCount >= MAX_FUNCTIONS) {
        putRError(node->lineNum);
        fprintf(stderr, "Function overflow.\n");
        exit(1);
    }

    strcpy(functions[functionCount].name, node->name);
    functions[functionCount].body = node->children[0];
    functionCount++;
}

ASTNode* getFunction(const char* name) {
    for (int i = 0; i < functionCount; i++) {
        if (strcmp(functions[i].name, name) == 0) {
            return functions[i].body;
        }
    }
    return NULL;
}

void pushScope() {
    if (scopeTop + 1 >= MAX_SCOPES) {
        fprintf(stderr, "RUNTIME Error: Scope overflow\n");
        exit(1);
    }
    scopeTop++;
    scopes[scopeTop].count = 0;
}

void popScope() {
    if (scopeTop <0) {
        fprintf(stderr, "RUNTIME Error: Scope underflow\n");
        exit(1);
    }
    scopeTop--;
}

Value* lookupVariable(const char* name) {
    for (int i = scopeTop; i >= 0; i--) {
        for (int j = 0; j < scopes[i].count; j++) {
            if (strcmp(scopes[i].vars[j].name, name) == 0) {
                return &scopes[i].vars[j].value;
            }
        }
    }
    return NULL;
}

void defineVariable(const char* name, Value val) {
    Scope* curr = &scopes[scopeTop];
    for (int i = 0; i < curr->count; i++) {
        if (strcmp(curr->vars[i].name, name) == 0) {
            fprintf(stderr, "RUNTIME Error: '%s' is already defined.\n", name);
            exit(1);
        }
    }
    if (curr->count >= MAX_VARIABLES) {
        fprintf(stderr, "RUNTIME Error: Variable overflow\n");
        exit(1);
    }
    strcpy(curr->vars[curr->count].name, name);
    curr->vars[curr->count].value = val;
    curr->count++;
}

void setVariable(const char* name, Value val) {
    Value* existing = lookupVariable(name);
    if (!existing) {
        fprintf(stderr, "RUNTIME Error: '%s' is undefined.\n", name);
        exit(1);
    }
    *existing = val;
}

// legacy
/*
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
*/
// end

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
            Value* v = lookupVariable(node->name);
            if (!v) {
                putRError(node->lineNum);
                fprintf(stderr, "Variable '%s' is undefined.\n", node->name);
                exit(1);
            }
            return *v;
        case AST_BINOP:
            Value left = eval(node->left);
            Value right = eval(node->right);
            if (left.type != TYPE_INT || right.type != TYPE_INT) {
                putRError(node->lineNum);
                fprintf(stderr, "Binary operation '%s' is undefined for type STRING.\n", node->value);
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
                    putRError(node->lineNum);
                    fprintf(stderr, "Unrecognized binary operator '%c'.\n", node->value[0]);
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
            Value defineValue = eval(node->right);
            defineVariable(node->name, defineValue);
            break;
        case AST_REDEFINE:
            Value redefValue = eval(node->right);
            setVariable(node->name, redefValue);
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
            pushScope();
            for (int i = 0; i < node->childCount; i++) {
                execStatement(node->children[i]);
            }
            popScope();
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
        case AST_FUNCTION:
            if (scopeTop != GLOBAL_SCOPE_INDEX) {
                putRError(node->lineNum);
                fprintf(stderr, "Function definitions are only allowed in the global scope.\n");
                exit(1);
            }
            defineFunction(node);
            break;
        case AST_CALL:
            ASTNode* funcBody = getFunction(node->value);
            if (!funcBody) {
                putRError(node->lineNum);
                fprintf(stderr, "Function '%s' is undefined.\n", node->value);
                exit(1);
            }
            execStatement(funcBody);
            break;
        case AST_EOL:
            // skip this jawn
            break;
        case AST_LEAVE:
            exit(atoi(node->value));
        case AST_LOOP:
            // If the loop count is a valid integer, run the loop.
            if (isInt(node->value)) {
                for (int i = 0; i < atoi(node->value); i++) {
                    execStatement(node->children[0]);
                }
            }
            break;
        default:
            putRError(node->lineNum);
            printf("Unrecognized node type '%s'.\n", nodename(node->type));
            exit(1);

    }
}

void interpret(ASTNode* root) {
    if (!root || root->type != AST_BLOCK) {
        putRError(root->lineNum);
        fprintf(stderr, "Program root must be BLOCK.\n");
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

bool isInt(const char* str) {
    if (*str == '-' || *str == '+') str++;
    if (*str == '\0') return false; // empty
    while (*str) {
        if (!isdigit(*str)) return false;
        str++;
    }
    return true;
}