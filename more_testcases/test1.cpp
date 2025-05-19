#include <iostream>
#include "tracker.h"

int main() {
    int* a = (int*)MALLOC(sizeof(int));
    int* b = (int*)CALLOC(2, sizeof(int));
    int* c = NEW int;
    int* d = NEW int;

    FREE(a);
    FREE(b);
    DELETE(c);
    DELETE(d);
    DELETE(d);  // double delete

    print_memory_report();
    return 0;
}
