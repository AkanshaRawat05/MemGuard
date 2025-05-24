#include <stdio.h>
#include "tracker.h"

int main() {
    // MALLOC allocated, never freed (leak)
    int* leak_ptr = (int*) MALLOC(sizeof(int));
    (void)leak_ptr; 
    
    // MALLOC allocated, freed properly
    int* normal_ptr = (int*) MALLOC(sizeof(int));
    FREE(normal_ptr);

    // CALLOC allocated, freed twice (double free warning)
    int* arr = (int*) CALLOC(5, sizeof(int));
    arr = (int*) REALLOC(arr, 10 * sizeof(int)); // reallocating
    FREE(arr);
    FREE(arr);  // double free warning expected here
    

    // NEW allocated, deleted properly
    int* new_ptr = NEW int;
    DELETE(new_ptr);

    // NEW allocated, deleted twice (double delete warning)
    int* new_ptr2 = NEW int;
    DELETE(new_ptr2);
   // DELETE(new_ptr2); // double delete warning expected here

    int* ptr = (int*) MALLOC(sizeof(int));
    *ptr = 100;

    // Check pointer before use — should be OK
    CHECK_PTR(ptr);
    FREE(ptr);
    // Check pointer after free — should warn about use-after-free
    CHECK_PTR(ptr);

    int* array = NEW_ARRAY int[10];    // Note: `new` calls with file and line info, then array size
    DELETE_ARRAY(array);
    DELETE_ARRAY(array);

    //print_memory_report();
    //print_memory_stats(); 
    //debug_head_location();

    export_json_report("memory_report.json");
    export_csv_report("memory_report.csv");


    return 0;
}


