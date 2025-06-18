#include "utils.h"

void putError(int n) {
    fprintf(stderr, "\nSYNTAX Error (Line %d): ", n);
}

void putRError(int n) {
    fprintf(stderr, "\nRUNTIME Error (Line %d): ", n);
}