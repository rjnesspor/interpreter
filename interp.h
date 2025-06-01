#define MAX_VARIABLES 20

typedef struct {
    char name[64];
    int value;
} Variable;

typedef struct {
    Variable* variables;
    int count;
} VariableTable;

Variable* getVariable(char* name);
void addVariable(Variable var);
void printVariables();