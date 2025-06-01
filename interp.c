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

    vars.variables = malloc(sizeof(Variable) * MAX_VARIABLES);

    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {

        if (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }


        char* tokens[256];
        int tokenCount = 0;
        char* token = strtok(line, " ");
        while (token != NULL) {
            tokens[tokenCount++] = token;
            token = strtok(NULL, " ");
        }

        // Integer x = 5
        if (strcmp(tokens[0], "Integer") == 0) {
            Variable new_variable;
            strcpy(new_variable.name, tokens[1]);
            new_variable.value = atoi(tokens[3]);
            addVariable(new_variable);
        }
        // Print x, Print "hi"
        else if (strcmp(tokens[0], "Print") == 0) {
            // String literal
            if (tokens[1][0] == '"') {

                for (int i = 1; i < tokenCount; i++) {
                    char* word = tokens[i];

                    if (i == 1 && word[0] == '"') {
                        word++;
                    }

                    int len = strlen(word);
                    if (word[len - 1] == '"') {
                        word[len - 1] = '\0';
                        printf("%s", word);
                        break;
                    } else {
                        printf("%s ", word);
                    }
                }

                puts("");

            } else {
                tokens[1][strcspn(tokens[1], "\n")] = 0;
                Variable* var = getVariable(tokens[1]);
                if (var) {
                    printf("%d\n", var->value);
                } else {
                    printf("null\n");
                }
            }
        }

    }

    //printVariables();

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

void printVariables() {
    for (int i = 0; i < vars.count; i++) {
        printf("Variable %s = %d\n", vars.variables[i].name, vars.variables[i].value);
    }
}