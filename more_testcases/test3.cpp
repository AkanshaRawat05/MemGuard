#include <stdio.h>
#include "tracker.h"

int main() {
    // MALLOC allocated, never freed (leak)
    int* leak_ptr = (int*) MALLOC(sizeof(int));

    // MALLOC allocated, freed properly
    int* normal_ptr = (int*) MALLOC(sizeof(int));
    FREE(normal_ptr);

    // CALLOC allocated, freed twice (double free warning)
    int* arr = (int*) CALLOC(5, sizeof(int));
    FREE(arr);
    FREE(arr);  // double free warning expected here

    // NEW allocated, deleted properly
    int* new_ptr = NEW int;
    DELETE(new_ptr);

    // NEW allocated, deleted twice (double delete warning)
    int* new_ptr2 = NEW int;
    DELETE(new_ptr2);
    DELETE(new_ptr2); // double delete warning expected here

    // Intentionally no FREE or DELETE on leak_ptr, so leak is detected

    print_memory_report();

    return 0;
}
