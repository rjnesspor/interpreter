#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

bool isInt(const char* str);
char* strConcat(char** array, int length);
void printError(char* message, int lineNum, char* fileName);

#endif