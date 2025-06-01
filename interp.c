#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "interp.h"

VariableTable vars;

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

    // We can hold a maximum of MAX_VARIABLES variables in the program memory
    vars.variables = malloc(sizeof(Variable) * MAX_VARIABLES);

    char line[256];
    int lineNumber = 1;
    while (fgets(line, sizeof(line), file) != NULL) {

        // Remove newline character
        if (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }

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

            Variable var;
            strncpy(var.name, varName, strlen(varName));

            if (strcmp(tokens[3], "as") != 0) {
                fprintf(stderr, "ERROR: (%s:%d) Expected 'as' to complete variable initialization, but got '%s'.\n", argv[1], lineNumber, tokens[3]);
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
                    for (int j = 0; j < strlen(str[i]); j++) {
                        finalString[index++] = str[i][j];
                        if (j + 1 == strlen(str[i])) {
                            finalString[index++] = ' ';
                        }
                    }
                }

                strncpy(var.value.stringValue, finalString, length);
                free(finalString);
                addVariable(var);

            } else {
                fprintf(stderr, "ERROR: (%s:%d) '%s' did not match any valid variable types.\n", argv[1], lineNumber, varType);
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

                fprintf(stderr, "ERROR: (%s:%d) '%s' did not match any valid exit statuses.\n",  argv[1], lineNumber, status);
                exit(1);

            }

        } else {

            fprintf(stderr, "ERROR: (%s:%d) Unrecognized directive '%s'.\n", argv[1], lineNumber, command);
            exit(1);

        }

        lineNumber++;

    }

    puts("\nReached the end of the program.\n");

    free(vars.variables);
    fclose(file);

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