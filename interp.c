#include "interp.h"

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

    while (fgets(line, sizeof(line), file) != NULL) {

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

        process(line, lineNumber, &executionEnabled);

        lineNumber++;

    }

    puts("\nReached the end of the program.\n");

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

                int length = 0;
                for (int i = 0; i < strTokens; i++) {
                    length += strlen(str[i]);
                }
                length += (strTokens - 1); // Add spaces
                length++; // Add null terminator

                char* finalString = malloc(length * sizeof(char));
                int index = 0;
                
                for (int i = 0; i < strTokens; i++) {
                    int wordLen = strlen(str[i]);
                    memcpy(&finalString[index], str[i], wordLen);
                    index += wordLen;
                    if (i < strTokens - 1) {
                        finalString[index++] = ' ';
                    }
                }

                finalString[index] = '\0';

                strncpy(var.value.stringValue, finalString, length);
                free(finalString);
                addVariable(var);

            } else {
                fprintf(stderr, "ERROR: (%s:%d) '%s' did not match any valid variable types.\n", fileName, lineNumber, varType);
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
            
        } else {

            // Check if we are trying to reassign a variable value
            Variable* varToChange = getVariable(command);
            if (varToChange) {            

                if (strcmp(tokens[1], "is") != 0) {
                    fprintf(stderr, "ERROR: (%s:%d) Expected 'is' to complete variable reassignment, but got '%s'.\n", fileName, lineNumber, tokens[1]);
                    exit(1);
                }

                // Math functions
                // x is x + 3
                // x is 5 + 5
                // x is 3 + x
                if (tokenCount == 5) {

                    char operator = tokens[3][0];

                    int arg1;
                    int arg2;
                    
                    // Parse arg1
                    if (isInt(tokens[2])) {
                        arg1 = atoi(tokens[2]);
                    } else {
                        Variable* v1 = getVariable(tokens[2]);
                        if (v1) {
                            if (v1->type == 0) {
                                arg1 = v1->value.intValue;
                            } else {
                                fprintf(stderr, "ERROR: (%s:%d) Cannot perform arithmetic on '%s' since it is of type STRING.\n", fileName, lineNumber, v1->name);
                                exit(1);
                            }
                        } else {
                            fprintf(stderr, "ERROR: (%s:%d) Variable '%s' is undefined.\n", fileName, lineNumber, tokens[2]);
                            exit(1);
                        }
                    }

                    // Parse arg2
                    if (isInt(tokens[4])) {
                        arg2 = atoi(tokens[4]);
                    } else {
                        Variable* v2 = getVariable(tokens[4]);
                        if (v2) {
                            if (v2->type == 0) {
                                arg2 = v2->value.intValue;
                            } else {
                                fprintf(stderr, "ERROR: (%s:%d) Cannot perform arithmetic on '%s' since it is of type STRING.\n", fileName, lineNumber, v2->name);
                                exit(1);
                            }
                        } else {
                            fprintf(stderr, "ERROR: (%s:%d) Variable '%s' is undefined.\n", fileName, lineNumber, tokens[4]);
                            exit(1);
                        }
                    }

                    int res;

                    if (operator == '+') {
                        res = arg1 + arg2;
                    } else if (operator == '-') {
                        res = arg1 - arg2;
                    } else if (operator == '*') {
                        res = arg1 * arg2;
                    } else if (operator == '/') {
                        res = arg1 / arg2;
                    } else {
                        fprintf(stderr, "ERROR: (%s:%d) Unrecognized arithmetic operator '%c'.\n", fileName, lineNumber, operator);
                        exit(1);
                    }

                    varToChange->value.intValue = res;

                } 
                // Reassignment
                // x is 5
                else if (tokenCount == 3) {

                    // TODO: check if an actual valid number is given 
                    varToChange->value.intValue = atoi(tokens[2]);

                }

            }

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

bool isInt(const char* str) {
    if (*str == '-' || *str == '+') str++;
    if (*str == '\0') return false; // empty
    while (*str) {
        if (!isdigit(*str)) return false;
        str++;
    }
    return true;
}