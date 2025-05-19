#include <stdio.h>
#include "tracker.h"

int main() {
    int* a = (int*) MALLOC(sizeof(int));      // allocated, never freed — leak
int* b = (int*) MALLOC(sizeof(int));      // allocated, freed properly
int* c = (int*) MALLOC(sizeof(int));
int* arr = (int*) CALLOC(10, sizeof(int)); // allocated, freed twice (double free warning)
    // allocated, freed twice (double free warning)

    FREE(b);       // correct free
    FREE(c);       // first free
    FREE(c);       // double free warning

    // Intentionally no FREE(a), so this leaks

    print_memory_report();

    return 0;
}
