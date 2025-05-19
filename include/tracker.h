#ifndef TRACKER_H
#define TRACKER_H

#ifdef __cplusplus
#include <cstddef>  // for std::size_t
#else
#include <stddef.h>
#endif

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
