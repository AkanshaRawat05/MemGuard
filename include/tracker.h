#ifndef TRACKER_H
#define TRACKER_H

#ifdef __cplusplus
#include <cstddef> // for std::size_t
#include "third_party/json.hpp"
using json = nlohmann::json;
#else
#include <stddef.h>
#endif

struct AllocationInfo {
    void* ptr = nullptr;
    size_t size = 0;
    std::string file;        // Source file where allocated
    int line = 0;            // Line number where allocated
    bool is_freed = false;   // Whether memory was freed
    bool is_array = false;   // true if allocated using new[]
    std::string free_file;   // Source file where freed
    int free_line = 0;       // Line number where freed

    bool double_free = false;       // True if this pointer was freed more than once
    bool dangling_pointer = false;  // True if this pointer was used after being freed

    AllocationInfo* next = nullptr; // Linked list pointer
};

extern AllocationInfo* head;
extern size_t total_allocations;
extern size_t total_frees;
extern size_t current_bytes;
extern size_t peak_bytes;

#ifdef __cplusplus
extern "C" {
#endif

void* my_calloc(size_t nmemb, size_t size, const char* file, int line);
void* my_malloc(size_t size, const char* file, int line);
void my_free(void* ptr, const char* file, int line);
void* my_realloc(void* ptr, size_t new_size, const char* file, int line);
void check_pointer(void* ptr, const char* file, int line);

// Add my_delete declaration (needed for DELETE macro)
void my_delete(void* ptr, const char* file, int line);

void* my_new_array(size_t size, const char* file, int line);
void my_delete_array(void* ptr, const char* file, int line);

void print_memory_report(void);
void print_memory_stats(void);
void export_json_report(const char* filename);
void export_csv_report(const char* filename);
void debug_head_location();

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
// C++ support for new/delete operator overloads
void* operator new(std::size_t size, const char* file, int line);
void operator delete(void* ptr) noexcept;
void operator delete(void* ptr, std::size_t size) noexcept;

void* operator new[](std::size_t size, const char* file, int line);
void operator delete[](void* ptr) noexcept;
void operator delete[](void* ptr, std::size_t size) noexcept;
#endif // __cplusplus

// Macros for usage
#define MALLOC(size) my_malloc(size, __FILE__, __LINE__)
#define CALLOC(nmemb, size) my_calloc(nmemb, size, __FILE__, __LINE__)
#define FREE(ptr) my_free(ptr, __FILE__, __LINE__)
#define REALLOC(ptr, size) my_realloc(ptr, size, __FILE__, __LINE__)
#define CHECK_PTR(ptr) check_pointer(ptr, __FILE__, __LINE__)

// For single objects
#define NEW new(__FILE__, __LINE__)
#define DELETE(ptr) my_delete(ptr, __FILE__, __LINE__)

// For arrays
#define NEW_ARRAY new(__FILE__, __LINE__)
#define DELETE_ARRAY(ptr) my_delete_array(ptr, __FILE__, __LINE__)

#endif // TRACKER_H


