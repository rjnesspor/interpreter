#include "interp.h"
#include "utils.h"

VariableTable vars;
char* fileName;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error opening file.");
        return 1;
    }
    fileName = argv[1];

    // We can hold a maximum of MAX_VARIABLES variables in the program memory
    vars.variables = malloc(sizeof(Variable) * MAX_VARIABLES);

    char line[256];
    int lineNumber = 1;
    
    // Flags
    bool executionEnabled = true;

    long loopStart = -1;
    int loopCount = 0;

    while (fgets(line, sizeof(line), file)) {

        // Remove newline character
        if (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }

        // Reached the end of our if statement, we can continue executing after this line.
        if (strcmp(line, "endif") == 0) {
            executionEnabled = true;
            continue;
        }

        // Skip this line if we are skipping an if statement.
        if (!executionEnabled) {
            continue;
        }

        // If this line is empty, skip it.
        if (strlen(line) == 0) {
            continue;
        }

        // Handle loops
        if (strncmp(line, "loop", 4) == 0) {
            if (!(line + 5) || !isInt((line + 5))) {
                fprintf(stderr, "ERROR: (%s:%d) Expected a loop count but got '%s'.\n", fileName, lineNumber, line + 5);
                exit(1);
            }
            loopCount = atoi(line + 5);
            loopStart = ftell(file);
        }

        if (strncmp(line, "endloop", 7) == 0) {
            if (--loopCount > 0 && loopStart != -1) {
                fseek(file, loopStart, SEEK_SET);
            }
        }

        process(line, lineNumber, &executionEnabled);

        lineNumber++;

    }

    puts("\nProgram exited (Dropped off bottom).\n");

    free(vars.variables);
    fclose(file);

}

void process(char* line, int lineNumber, bool* executionEnabled) {

        // Tokenize line by " "
        char* tokens[256];
        int tokenCount = 0;
        char* token = strtok(line, " ");
        while (token != NULL) {
            tokens[tokenCount++] = token;
            token = strtok(NULL, " ");
        }

        char* command = tokens[0];

        // We are declaring a new variable
        if (strcmp(command, "define") == 0) {

            char* varType = tokens[1];
            char* varName = tokens[2];

            Variable* v = getVariable(varName);
            if (v) {
                fprintf(stderr, "ERROR: (%s:%d) '%s' has already been defined.\n", fileName, lineNumber, varName);
                exit(1);
            }

            Variable var;
            strncpy(var.name, varName, strlen(varName) + 1);

            if (strcmp(tokens[3], "as") != 0) {
                fprintf(stderr, "ERROR: (%s:%d) Expected 'as' to complete variable initialization, but got '%s'.\n", fileName, lineNumber, tokens[3]);
                exit(1);
            }

            if (strcmp(varType, "integer") == 0) {
                // define integer x as 15

                var.type = 0;
                var.value.intValue = atoi(tokens[4]);  
                addVariable(var);       

            } else if (strcmp(varType, "string") == 0) {
                // define string y as hello, world!

                var.type = 1;

                // I have no idea what this entire branch does but it works

                int strTokens = tokenCount - 4;
                char** str = (tokens + 4);

                char* finalString = strConcat(str, strTokens);

                strncpy(var.value.stringValue, finalString, strlen(finalString) + 1);
                free(finalString);
                addVariable(var);

            } else {
                fprintf(stderr, "ERROR: (%s:%d) Expected a variable type, but got '%s'.\n", fileName, lineNumber, varType);
                exit(1);
            }

        }
        // We are printing a variable
        // print x
        else if (strcmp(command, "print") == 0) {

            Variable* var = getVariable(tokens[1]);
            printVariable(var);

        }
        // We are exiting the program with a status
        // exit success
        else if (strcmp(command, "leave") == 0) {

            char* status = tokens[1];

            if (strcmp(status, "success") == 0) {

                puts("\nProgram exited with status SUCCESS.\n");
                exit(0);

            } else if (strcmp(status, "error") == 0) {

                puts("\nProgram exited with status ERROR.\n");
                exit(1);

            } else {

                fprintf(stderr, "ERROR: (%s:%d) '%s' did not match any valid exit statuses.\n", fileName, lineNumber, status);
                exit(1);

            }

        } 
        // If statement
        // if x = 5
        // if y > 2
        else if (strcmp(command, "if") == 0) {
            
            char operator = tokens[2][0];
            
            int arg1;
            int arg2;

            // Parse arg1
            if (isInt(tokens[1])) {
                arg1 = atoi(tokens[1]);
            } else {
                Variable* v1 = getVariable(tokens[1]);
                if (v1) {
                    if (v1->type == 0) {
                        arg1 = v1->value.intValue;
                    } else {
                        fprintf(stderr, "ERROR: (%s:%d) Cannot perform comparison on '%s' since it is of type STRING.\n", fileName, lineNumber, v1->name);
                        exit(1);
                    }
                } else {
                    fprintf(stderr, "ERROR: (%s:%d) Variable '%s' is undefined.\n", fileName, lineNumber, tokens[1]);
                    exit(1);
                }
            }

            // Parse arg2
            if (isInt(tokens[3])) {
                arg2 = atoi(tokens[3]);
            } else {
                Variable* v2 = getVariable(tokens[3]);
                if (v2) {
                    if (v2->type == 0) {
                        arg2 = v2->value.intValue;
                    } else {
                        fprintf(stderr, "ERROR: (%s:%d) Cannot perform comparison on '%s' since it is of type STRING.\n", fileName, lineNumber, v2->name);
                        exit(1);
                    }
                } else {
                    fprintf(stderr, "ERROR: (%s:%d) Variable '%s' is undefined.\n", fileName, lineNumber, tokens[3]);
                    exit(1);
                }
            }

            if (operator == '=') {
                if (arg1 != arg2) {
                    *executionEnabled = false;
                }
            } else if (operator == '>') {
                if (arg1 <= arg2) {
                    *executionEnabled = false;
                }
            } else if (operator == '<') {
                if (arg1 >= arg2) {
                    *executionEnabled = false;
                }
            } else {
                fprintf(stderr, "ERROR: (%s:%d) Unrecognized comparison operator '%c'.\n", fileName, lineNumber, operator);
                exit(1);
            }
            
        }
        // redefine x as integer 5
        // redefine y as string hello, world!
        // redefine z as x + y
        else if (strcmp(command, "redefine") == 0) {

            Variable* varToChange = getVariable(tokens[1]);

            if (varToChange) {

                if (strcmp(tokens[2], "as") != 0) {
                    fprintf(stderr, "ERROR: (%s:%d) Expected 'as' to complete variable reassignment, but got '%s'.\n", fileName, lineNumber, tokens[2]);
                    exit(1);
                }

                if (strcmp(tokens[3], "integer") == 0) {

                    if (isInt(tokens[4])) {

                        varToChange->type = 0;
                        varToChange->value.intValue = atoi(tokens[4]);

                    }

                } else if (strcmp(tokens[3], "string") == 0) {

                    char** array = (tokens + 4);
                    int arrayLength = tokenCount - 4;

                    char* finalString = strConcat(array, arrayLength);

                    varToChange->type = 1;
                    strncpy(varToChange->value.stringValue, finalString, strlen(finalString) + 1);
                    free(finalString);

                } else {
                    // Check if we are trying to do math

                    int result = parseIntegerMath(tokens[3], tokens[5], tokens[4][0], lineNumber);
                    
                    varToChange->type = 0;
                    varToChange->value.intValue = result;

                }

            } else {
                fprintf(stderr, "ERROR: (%s:%d) Variable '%s' is undefined.\n", fileName, lineNumber, tokens[1]);
                exit(1);
            }

        } 
        // input integer as z
        // input string as y
        else if (strcmp(command, "input") == 0) {

            if (strcmp(tokens[2], "as") != 0) {
                fprintf(stderr, "ERROR: (%s:%d) Expected 'as' to complete variable assignment, but got '%s'.\n", fileName, lineNumber, tokens[2]);
                exit(1);
            }

            char* type = tokens[1];
            if (strcmp(type, "integer") == 0) {

                if (getVariable(tokens[3])) {
                    Variable* varToChange = getVariable(tokens[3]);
                    varToChange->type = 0;
                    scanf("%d", &varToChange->value.intValue);
                } else {
                    Variable newVar;
                    strncpy(newVar.name, tokens[3], strlen(tokens[3]) + 1);
                    newVar.type = 0;
                    scanf("%d", &newVar.value.intValue);
                    addVariable(newVar);
                }

                
            } else if (strcmp(type, "string") == 0) {

                if (getVariable(tokens[3])) {
                    Variable* varToChange = getVariable(tokens[3]);
                    varToChange->type = 1;
                    scanf("%127[^\n]", varToChange->value.stringValue);
                } else {
                    Variable newVar;
                    strncpy(newVar.name, tokens[3], strlen(tokens[3]) + 1);
                    newVar.type = 1;
                    scanf("%127[^\n]", newVar.value.stringValue);
                    addVariable(newVar);
                }

            } else {
                fprintf(stderr, "ERROR: (%s:%d) Expected a variable type, but got '%s'.\n", fileName, lineNumber, type);
                exit(1);
            }

        } 
        // loop 10
        else if (strcmp(command, "loop") == 0 || strcmp(command, "endloop") == 0) {

            // do nothing

        } else {
            fprintf(stderr, "ERROR: (%s:%d) Unrecognized directive '%s'.\n", fileName, lineNumber, command);
            exit(1);
        }

}

Variable* getVariable(char* name) {
    for (int i = 0; i < vars.count; i++) {
        if (strcmp(vars.variables[i].name, name) == 0) {
            return &vars.variables[i];
        }
    }
    return NULL;
}

void addVariable(Variable var) {
    vars.variables[vars.count] = var;
    vars.count++;
}

void printVariable(Variable* var) {

    if (!var) {
        printf("null\n");
    } else {
        switch (var->type) {

            case 0:
                printf("%d\n", var->value.intValue);
                break;
            case 1:
                printf("%s\n", var->value.stringValue);
                break;
            default:
                printf("null\n");
                break;

        }
    }

}

void printVariables() {
    for (int i = 0; i < vars.count; i++) {
        printVariable(&vars.variables[i]);
    }
}



int parseIntegerMath(char* arg1, char* arg2, char operator, int lineNumber) {
    int parsedArg1;
    int parsedArg2;
    
    // Parse arg1
    if (isInt(arg1)) {
        parsedArg1 = atoi(arg1);
    } else {
        Variable* v1 = getVariable(arg1);
        if (v1) {
            if (v1->type == 0) {
                parsedArg1 = v1->value.intValue;
            } else {
                fprintf(stderr, "ERROR: (%s:%d) Cannot perform arithmetic on '%s' since it is of type STRING.\n", fileName, lineNumber, v1->name);
                exit(1);
            }
        } else {
            fprintf(stderr, "ERROR: (%s:%d) Variable '%s' is undefined.\n", fileName, lineNumber, arg1);
            exit(1);
        }
    }

    // Parse arg2
    if (isInt(arg2)) {
        parsedArg2 = atoi(arg2);
    } else {
        Variable* v2 = getVariable(arg2);
        if (v2) {
            if (v2->type == 0) {
                parsedArg2 = v2->value.intValue;
            } else {
                fprintf(stderr, "ERROR: (%s:%d) Cannot perform arithmetic on '%s' since it is of type STRING.\n", fileName, lineNumber, v2->name);
                exit(1);
            }
        } else {
            fprintf(stderr, "ERROR: (%s:%d) Variable '%s' is undefined.\n", fileName, lineNumber, arg2);
            exit(1);
        }
    }

    int res;

    if (operator == '+') {
        res = parsedArg1 + parsedArg2;
    } else if (operator == '-') {
        res = parsedArg1 - parsedArg2;
    } else if (operator == '*') {
        res = parsedArg1 * parsedArg2;
    } else if (operator == '/') {
        res = parsedArg1 / parsedArg2;
    } else {
        fprintf(stderr, "ERROR: (%s:%d) Unrecognized arithmetic operator '%c'.\n", fileName, lineNumber, operator);
        exit(1);
    }

    return res;

}