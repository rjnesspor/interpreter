#include "interpreter.h"

Scope scopes[MAX_SCOPES];
int scopeTop = -1;

Function functions[MAX_FUNCTIONS];
int functionCount = 0;

CallFrame callStack[MAX_SCOPES];
int callTop = -1;

BinopRule binopRules[] = {
    { TYPE_INT, TYPE_INT, "+", int_int_add },
    { TYPE_INT, TYPE_INT, "-", int_int_sub },
    { TYPE_INT, TYPE_INT, "*", int_int_mul },
    { TYPE_INT, TYPE_INT, "/", int_int_div },
    { TYPE_INT, TYPE_INT, "%", int_int_mod },
    { TYPE_INT, TYPE_INT, "=", int_int_eq },
    { TYPE_INT, TYPE_INT, "<", int_int_lt },
    { TYPE_INT, TYPE_INT, ">", int_int_gt },
    { TYPE_INT, TYPE_INT, ">=", int_int_gte },
    { TYPE_INT, TYPE_INT, "<=", int_int_lte },
    { TYPE_INT, TYPE_INT, "!=", int_int_neq },
    { TYPE_FLOAT, TYPE_FLOAT, "+", float_float_add },
    { TYPE_FLOAT, TYPE_FLOAT, "-", float_float_sub },
    { TYPE_FLOAT, TYPE_FLOAT, "*", float_float_mul },
    { TYPE_FLOAT, TYPE_FLOAT, "/", float_float_div },
    { TYPE_FLOAT, TYPE_FLOAT, "=", float_float_eq },
    { TYPE_FLOAT, TYPE_FLOAT, "<", float_float_lt },
    { TYPE_FLOAT, TYPE_FLOAT, ">", float_float_gt },
    { TYPE_FLOAT, TYPE_FLOAT, ">=", float_float_gte },
    { TYPE_FLOAT, TYPE_FLOAT, "<=", float_float_lte },
    { TYPE_FLOAT, TYPE_FLOAT, "!=", float_float_neq },
    { TYPE_STRING, TYPE_STRING, "+", string_string_add },
    { TYPE_STRING, TYPE_STRING, "=", string_string_eq },
    { TYPE_STRING, TYPE_STRING, "!=", string_string_neq },
    { TYPE_STRING, TYPE_STRING, "<", string_string_lt },
    { TYPE_STRING, TYPE_STRING, ">", string_string_gt },
};

int ruleCount = sizeof(binopRules) / sizeof(BinopRule);

void pushCallFrame(ASTNode* function) {
    if (callTop + 1 >= MAX_SCOPES) {
        putRError(function->lineNum);
        fprintf(stderr, "Call stack overflow.\n");
        exit(1);
    }
    callTop++;
    callStack[callTop].function = function;
    callStack[callTop].hasReturned = 0;
    callStack[callTop].scopeBase = scopeTop;
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
    // Floor refers to which scope was in effect when this call frame
    // was pushed, so that we do not traverse into scopes from other (recursive)
    // function calls, and still traverse the global scope
    int floor = (callTop >= 0) ? callStack[callTop].scopeBase : 0;
    for (int i = scopeTop; i >= floor; i--) {
        for (int j = 0; j < scopes[i].count; j++) {
            if (strcmp(scopes[i].vars[j].name, name) == 0) {
                return &scopes[i].vars[j].value;
            }
        }
    }
    // Still traverse the global scope
    if (floor > 0) {
        for (int j = 0; j < scopes[0].count; j++) {
            if (strcmp(scopes[0].vars[j].name, name) == 0) {
                return &scopes[0].vars[j].value;
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
            char* op = node->value;

            for (int i = 0; i < ruleCount; i++) {
                BinopRule* rule = &binopRules[i];
                if (rule->left == left.typeDesc->base && rule->right == right.typeDesc->base && strcmp(rule->op, op) == 0) {
                    return rule->handler(left, right);
                }
            }
            putRError(node->lineNum);
            fprintf(stderr, "Binary operation '%s' is undefined for types '%s' and '%s'.\n", op, typeName(left.typeDesc), typeName(right.typeDesc));
            exit(1);
        case AST_LIST:
            Value list;
            list.arrayValue.length = node->childCount;
            list.arrayValue.elements = malloc(sizeof(Value) * node->childCount);
            TypeDesc* elemType = typeInt();
            for (int i = 0; i < node->childCount; i++) {
                Value elem = eval(node->children[i]);
                if (i == 0) elemType = elem.typeDesc;
                else if (!typeEquals(elem.typeDesc, elemType)) {
                    putRError(node->lineNum);
                    fprintf(stderr, "List elements must all be the same type.\n");
                    exit(1);
                }
                list.arrayValue.elements[i] = elem;
            }
            list.typeDesc = typeList(elemType);
            return list;
        case AST_INDEX:
            Value* listVar = lookupVariable(node->name);
            if (!listVar) {
                putRError(node->lineNum);
                fprintf(stderr, "Variable '%s' is undefined.\n", node->name);
                exit(1);
            }
            if (listVar->typeDesc->base != TYPE_LIST) {
                putRError(node->lineNum);
                fprintf(stderr, "'%s' is not a list.\n", node->name);
                exit(1);
            }
            Value idxVal = eval(node->right);
            if (idxVal.typeDesc->base != TYPE_INT) {
                putRError(node->lineNum);
                fprintf(stderr, "List index must be an integer.\n");
                exit(1);
            }
            if (idxVal.intValue < 0 || idxVal.intValue >= listVar->arrayValue.length) {
                putRError(node->lineNum);
                fprintf(stderr, "List index %d out of bounds for '%s' (length %d).\n", idxVal.intValue, node->name, listVar->arrayValue.length);
                exit(1);
            }
            return listVar->arrayValue.elements[idxVal.intValue];
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
            if (node->left) {
                Value* redefList = lookupVariable(node->name);
                if (!redefList) {
                    putRError(node->lineNum);
                    fprintf(stderr, "'%s' is undefined.\n", node->name);
                    exit(1);
                }
                if (redefList->typeDesc->base != TYPE_LIST) {
                    putRError(node->lineNum);
                    fprintf(stderr, "'%s' is not a list.\n", node->name);
                    exit(1);
                }
                Value redefIdx = eval(node->left);
                if (redefIdx.typeDesc->base != TYPE_INT) {
                    putRError(node->lineNum);
                    fprintf(stderr, "List index must be an integer.\n");
                    exit(1);
                }
                if (redefIdx.intValue < 0 || redefIdx.intValue >= redefList->arrayValue.length) {
                    putRError(node->lineNum);
                    fprintf(stderr, "List index %d out of bounds for '%s' (length %d).\n", redefIdx.intValue, node->name, redefList->arrayValue.length);
                    exit(1);
                }
                Value redefElem = eval(node->right);
                if (!typeEquals(redefElem.typeDesc, redefList->typeDesc->elementType)) {
                    putRError(node->lineNum);
                    fprintf(stderr, "Type mismatch: cannot assign '%s' value into list '%s' of '%s'.\n", typeName(redefElem.typeDesc), node->name, typeName(redefList->typeDesc->elementType));
                    exit(1);
                }
                redefList->arrayValue.elements[redefIdx.intValue] = redefElem;
                break;
            }
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
                printf("%g\n", printVal.floatValue);
            } else {
                putRError(node->lineNum);
                fprintf(stderr, "Unknown type: %s.\n", typeName(printVal.typeDesc));
                exit(1);
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

            // evaluate the arguments before we enter the new scope
            // since the call base will stop us from evaluating them after
            // it is pushed
            Value argValues[MAX_PARAMS];
            for (int i = 0; i < func->paramCount; i++) {
                argValues[i] = eval(args[i]);
            }

            pushScope();
            pushCallFrame(func);

            for (int i = 0; i < func->paramCount; i++) {
                defineVariable(params[i]->name, params[i]->typeDesc, argValues[i]);
            }

            execStatement(funcBody);

            if (strlen(node->returnVar) > 0 && !callStack[callTop].hasReturned) {
                putRError(node->lineNum);
                fprintf(stderr, "Function '%s' returned no value but attempts to assign result to '%s'.\n", func->name, node->returnVar);
                exit(1);
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
                }
                frame->hasReturned = 1;
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
        case AST_LIST:      return "LIST";
        case AST_INDEX:     return "INDEX";
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

#define ARITH_BINOP(NAME, FIELD, TYPEFN, OP) \
    Value NAME(Value left, Value right) { \
        Value v; \
        v.typeDesc = TYPEFN(); \
        v.FIELD = left.FIELD OP right.FIELD; \
        return v; \
    }

#define CMP_BINOP(NAME, FIELD, OP) \
    Value NAME(Value left, Value right) { \
        Value v; \
        v.typeDesc = typeInt(); \
        v.intValue = left.FIELD OP right.FIELD; \
        return v; \
    }

ARITH_BINOP(int_int_add, intValue, typeInt, +)
ARITH_BINOP(int_int_sub, intValue, typeInt, -)
ARITH_BINOP(int_int_mul, intValue, typeInt, *)
ARITH_BINOP(int_int_div, intValue, typeInt, /)
ARITH_BINOP(int_int_mod, intValue, typeInt, %)
CMP_BINOP(int_int_eq, intValue, ==)
CMP_BINOP(int_int_neq, intValue, !=)
CMP_BINOP(int_int_lt, intValue, <)
CMP_BINOP(int_int_gt, intValue, >)
CMP_BINOP(int_int_gte, intValue, >=)
CMP_BINOP(int_int_lte, intValue, <=)

ARITH_BINOP(float_float_add, floatValue, typeFloat, +)
ARITH_BINOP(float_float_sub, floatValue, typeFloat, -)
ARITH_BINOP(float_float_mul, floatValue, typeFloat, *)
ARITH_BINOP(float_float_div, floatValue, typeFloat, /)
CMP_BINOP(float_float_eq, floatValue, ==)
CMP_BINOP(float_float_neq, floatValue, !=)
CMP_BINOP(float_float_lt, floatValue, <)
CMP_BINOP(float_float_gt, floatValue, >)
CMP_BINOP(float_float_gte, floatValue, >=)
CMP_BINOP(float_float_lte, floatValue, <=)

#undef ARITH_BINOP
#undef CMP_BINOP

Value string_string_add(Value left, Value right) {
    Value v;
    snprintf(v.strValue, sizeof(v.strValue), "%s%s", left.strValue, right.strValue);
    v.typeDesc = typeString();
    return v;
}

Value string_string_eq(Value left, Value right) {
    Value v = { .typeDesc = typeInt(), .intValue = !strcmp(left.strValue, right.strValue) };
    return v;
}

Value string_string_neq(Value left, Value right) {
    Value v = { .typeDesc = typeInt(), .intValue = strcmp(left.strValue, right.strValue) != 0 };
    return v;
}

Value string_string_lt(Value left, Value right) {
    Value v = { .typeDesc = typeInt(), .intValue = strcmp(left.strValue, right.strValue) < 0 };
    return v;
}

Value string_string_gt(Value left, Value right) {
    Value v = { .typeDesc = typeInt(), .intValue = strcmp(left.strValue, right.strValue) > 0 };
    return v;
}