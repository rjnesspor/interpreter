#include "interpreter.h"

Scope scopes[MAX_SCOPES];
int scopeTop = -1;

Function functions[MAX_FUNCTIONS];
int functionCount = 0;

CallFrame callStack[MAX_SCOPES];
int callTop = -1;

void pushCallFrame(ASTNode* function) {
    if (callTop + 1 >= MAX_SCOPES) {
        putRError(function->lineNum);
        fprintf(stderr, "Call stack overflow.\n");
        exit(1);
    }
    callTop++;
    callStack[callTop].function = function;
    callStack[callTop].hasReturned = 0;
}

void popCallFrame() {
    if (callTop < 0) {
        fprintf(stderr, "RUNTIME Error: Call stack underflow.\n");
        exit(1);
    }
    callTop--;
}

CallFrame* currentCallFrame() {
    return callTop >= 0 ? &callStack[callTop] : NULL;
}

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
    functions[functionCount].body = node;
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
        fprintf(stderr, "RUNTIME Error: Scope overflow.\n");
        exit(1);
    }
    scopeTop++;
    scopes[scopeTop].count = 0;
}

void popScope() {
    if (scopeTop <0) {
        fprintf(stderr, "RUNTIME Error: Scope underflow.\n");
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

void defineVariable(const char* name, TypeDesc* declaredType, Value val) {
    Scope* curr = &scopes[scopeTop];
    for (int i = 0; i < curr->count; i++) {
        if (strcmp(curr->vars[i].name, name) == 0) {
            fprintf(stderr, "\nRUNTIME Error: '%s' is already defined.\n", name);
            exit(1);
        }
    }
    if (curr->count >= MAX_VARIABLES) {
        fprintf(stderr, "\nRUNTIME Error: Variable overflow\n");
        exit(1);
    }
    if (!typeEquals(declaredType, val.typeDesc)) {
        fprintf(stderr, "\nRUNTIME Error: Type mismatch: cannot assign '%s' value to variable '%s' which is of type '%s'.\n", typeName(val.typeDesc), name, typeName(declaredType));
        exit(1);
    }
    strcpy(curr->vars[curr->count].name, name);
    curr->vars[curr->count].value = val;
    curr->count++;
}

void setVariable(const char* name, Value val) {
    Value* existing = lookupVariable(name);
    if (!existing) {
        fprintf(stderr, "\nRUNTIME Error: '%s' is undefined.\n", name);
        exit(1);
    }
    if (!typeEquals(val.typeDesc, existing->typeDesc)) {
        fprintf(stderr, "\nRUNTIME Error: Type mismatch: cannot assign '%s' value to variable '%s' which is of type '%s'.\n", typeName(val.typeDesc), name, typeName(existing->typeDesc));
        exit(1);
    }
    *existing = val;
}

Value eval(ASTNode* node) {
    switch(node->type) {
        case AST_LITERAL:
            Value val;
            if (typeEquals(node->typeDesc, typeInt())) {
                val.typeDesc = typeInt();
                val.intValue = atoi(node->value);
            } else if (typeEquals(node->typeDesc, typeString())) {
                val.typeDesc = typeString();
                strcpy(val.strValue, node->value);
            } else if (typeEquals(node->typeDesc, typeFloat())) {
                val.typeDesc = typeFloat();
                val.floatValue = atof(node->value);
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
            if (!typeEquals(left.typeDesc, typeInt()) || !typeEquals(right.typeDesc, typeInt())) {
                putRError(node->lineNum);
                fprintf(stderr, "Binary operation '%s' is undefined for operands '%s' and '%s'.\n", node->value, typeName(left.typeDesc), typeName(right.typeDesc));
                exit(1);
            }
            Value result;
            result.typeDesc = typeInt();
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
            printf("Something went wrong with node: %s", node->name);
            exit(1);
    }
}

void execStatement(ASTNode* node) {
    switch (node->type) {
        case AST_DEFINE:
            Value defineValue = eval(node->right);
            defineVariable(node->name, node->typeDesc, defineValue);
            break;
        case AST_REDEFINE:
            Value redefValue = eval(node->right);
            setVariable(node->name, redefValue);
            break;
        case AST_PRINT:
            Value printVal = eval(node->right);
            if (typeEquals(printVal.typeDesc, typeInt())) {
                printf("%d\n", printVal.intValue);
            } else if (typeEquals(printVal.typeDesc, typeString())) {
                printf("%s\n", printVal.strValue);
            } else if (typeEquals(printVal.typeDesc, typeFloat())) {
                printf("%f\n", printVal.floatValue);
            }
            break;
        case AST_BLOCK:
            pushScope();
            for (int i = 0; i < node->childCount; i++) {
                execStatement(node->children[i]);
                if (callTop >= 0 && callStack[callTop].hasReturned) break;
            }
            popScope();
            break;
        case AST_INPUT:
            Value inputVal;
            if (typeEquals(node->typeDesc, typeInt())) {
                inputVal.typeDesc = typeInt();
                fscanf(stdin, "%d", &inputVal.intValue);
                setVariable(node->name, inputVal);
            } else if (typeEquals(node->typeDesc, typeString())) {
                inputVal.typeDesc = typeString();
                fscanf(stdin, "%[^\n]", &inputVal.strValue);
                setVariable(node->name, inputVal);
            } else if (typeEquals(node->typeDesc, typeFloat())) {
                inputVal.typeDesc = typeFloat();
                fscanf(stdin, "%f", &inputVal.floatValue);
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
            ASTNode* func = getFunction(node->value);

            if (!func) {
                putRError(node->lineNum);
                fprintf(stderr, "Function '%s' is undefined.\n", node->value);
                exit(1);
            }

            ASTNode** params = func->parameters;
            ASTNode** args = node->args;

            ASTNode* funcBody = func->children[0];

            if (func->paramCount != node->argCount) {
                putRError(node->lineNum);
                fprintf(stderr, "Function '%s' expects %d arguments, got %d.\n", node->value, func->paramCount, node->argCount);
                exit(1);
            }

            for (int i = 0; i < func->paramCount; i++) {
                Value* v = lookupVariable(args[i]->name);
                if (!v) {
                    putRError(node->lineNum);
                    fprintf(stderr, "Variable '%s' is undefined.\n", args[i]->name);
                    exit(1);
                }
                if (typeEquals(params[i]->typeDesc, v->typeDesc)) continue;
                putRError(node->lineNum);
                fprintf(stderr, "Parameter %d of function '%s' expects type of '%s', but got '%s'.\n", i + 1, node->value, typeName(params[i]->typeDesc), typeName(v->typeDesc));
                exit(1);
            }

            pushScope();
            pushCallFrame(func);

            for (int i = 0; i < func->paramCount; i++) {
                Value val = eval(args[i]);
                defineVariable(params[i]->name, params[i]->typeDesc, val);
            }

            execStatement(funcBody);

            if (strlen(node->returnVar) > 0 && !callStack[callTop].hasReturned) {
                putWarning(node->lineNum);
                fprintf(stderr, "Function '%s' returned no value but assigns result to '%s'.\n", func->name, node->returnVar);
            }
            Value returnValue = callStack[callTop].returnValue;

            popCallFrame();
            popScope();

            if (strlen(node->returnVar) > 0) {
                if (lookupVariable(node->returnVar)) {
                    setVariable(node->returnVar, returnValue);
                } else {
                    defineVariable(node->returnVar, returnValue.typeDesc, returnValue);
                }
            }
            break;
        case AST_EOL:
            break;
        case AST_LEAVE:
            CallFrame* frame = currentCallFrame();
            if (frame) {
                // inside a function, so we want to return a value
                if (node->right) {
                    frame->returnValue = eval(node->right);
                    frame->hasReturned = 1;
                }
            } else {
                // not inside a function, exit accordingly
                int integerStatus = 0;
                if (node->right) {
                    Value status = eval(node->right);
                    integerStatus = typeEquals(status.typeDesc, typeInt()) ? status.intValue : 0;
                }
                exit(integerStatus);
            }
            break;
        case AST_LOOP:
            if (node->right) {
                Value count = eval(node->right);
                if (!typeEquals(count.typeDesc, typeInt())) {
                    putRError(node->lineNum);
                    printf("Loop count must be an integer, but encountered a(n) '%s'.\n", typeName(count.typeDesc));
                    exit(1);
                }
                for (int i = 0; i < count.intValue; i++) {
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