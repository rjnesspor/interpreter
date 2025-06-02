#include "utils.h"

bool isInt(const char* str) {
    if (*str == '-' || *str == '+') str++;
    if (*str == '\0') return false; // empty
    while (*str) {
        if (!isdigit(*str)) return false;
        str++;
    }
    return true;
}

char* strConcat(char** array, int arrayLength) {

    int length = 0;
    for (int i = 0; i < arrayLength; i++) {
        length += strlen(array[i]);
    }
    length += (arrayLength - 1); // Add spaces
    length++; // Add null terminator

    char* finalString = malloc(length * sizeof(char));
    int index = 0;
    
    for (int i = 0; i < arrayLength; i++) {
        int wordLen = strlen(array[i]);
        memcpy(&finalString[index], array[i], wordLen);
        index += wordLen;
        if (i < arrayLength - 1) {
            finalString[index++] = ' ';
        }
    }

    finalString[index] = '\0';
    return finalString;

}