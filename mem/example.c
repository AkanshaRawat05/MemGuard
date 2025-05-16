#include <stdio.h>
#include "leak.h"

int main() {
    // 1. Proper allocation and deallocation (OK)
    char *a = malloc(100);
    free(a);

    // 2. Memory Leak: Allocation without free
    char *b = malloc(120);  // --> LEAK (120)

    // 3. Memory Leak: Overwriting reference without freeing first
    char *c = calloc(10, 8); // allocates 80 bytes
    c = malloc(10);          // --> LEAK (80)

    // 4. Memory Leak: Realloc frees old pointer and allocates new, but old ref lost
    char *e = malloc(50);
    char *f = realloc(e, 100);  // OK if f is used
    // We forget to free f --> LEAK (100)

    // // 5. Double Free Error
    // char *g = malloc(30);
    // free(g);
    // // This will terminate the program with error
    // // Uncomment to see the result
    // // free(g); // --> Double Free Detected

    // 6. Use After Free (Dangling Pointer)
    char *h = malloc(60);
    free(h);
    check_dangling(h);  // --> Warning for dangling pointer
    // Unsafe access after free
    // Uncomment to test: h[0] = 'x';

    // 7. NULL pointer free warning
    char *i = NULL;
    free(i); // --> Warning for freeing NULL

    // 5. Double Free Error
    char *g = malloc(30);
    free(g);
    // This will terminate the program with error
    // Uncomment to see the result
    free(g); // --> Double Free Detected

    // Final result will be printed automatically on program exit by atexit
    return 0;
}
