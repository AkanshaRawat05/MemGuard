#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>  // for std::nothrow
#include "tracker.h"

// ANSI color codes
#define COLOR_RESET   "\x1b[0m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"

struct AllocationInfo {
    void* ptr;
    size_t size;
    char* file;
    int line;
    bool is_freed;
    bool is_array;  // true if allocated by new[]
    char* free_file;
    int free_line;

    AllocationInfo* next;
};


static AllocationInfo* head = NULL;
size_t total_allocations = 0;
size_t total_frees = 0;
size_t current_bytes = 0;
size_t peak_bytes = 0;

void* my_malloc(size_t size, const char* file, int line) {
    void* ptr = malloc(size);
    if (!ptr) return NULL;

    AllocationInfo* info = (AllocationInfo*)malloc(sizeof(AllocationInfo));
    if (!info) {
        free(ptr);
        return NULL;
    }

    info->ptr = ptr;
    info->size = size;
    info->file = strdup(file);
    if (!info->file) {
        free(ptr);
        free(info);
        return NULL;
    }

    info->line = line;
    info->is_freed = 0;
    info->next = head;
    head = info;

    printf(COLOR_GREEN "[ALLOC] %zu bytes → %p (at %s:%d)\n" COLOR_RESET, size, ptr, file, line);
    total_allocations++;
current_bytes += size;
if (current_bytes > peak_bytes) peak_bytes = current_bytes;


    return ptr;
}
void my_free(void* ptr, const char* file, int line) {
    if (!ptr) {
        printf(COLOR_YELLOW "[FREE] Warning: Attempt to free NULL pointer (at %s:%d)\n" COLOR_RESET, file, line);
        return;
    }

    AllocationInfo* current = head;
    while (current) {
        if (current->ptr == ptr) {
            if (current->is_freed) {
                printf(COLOR_RED "[FREE] Warning: Double free detected on %p (originally freed at %s:%d) (attempted at %s:%d)\n" COLOR_RESET,
                       ptr,
                       current->free_file ? current->free_file : "unknown",
                       current->free_line,
                       file, line);
                return;
            }

            printf(COLOR_YELLOW "[FREE] %p freed (originally allocated at %s:%d) (freed at %s:%d)\n" COLOR_RESET,
                   ptr, current->file, current->line, file, line);

            current->is_freed = 1;
            current_bytes -= current->size;
            total_frees++;

            // Save free location info for future double free detection
            if (current->free_file) free(current->free_file);  // free old if any
            current->free_file = strdup(file);
            current->free_line = line;

            free(ptr);
            return;
        }
        current = current->next;
    }

    printf(COLOR_RED "[FREE] Warning: Attempt to free untracked pointer %p (at %s:%d)\n" COLOR_RESET, ptr, file, line);
}


void* my_calloc(size_t nmemb, size_t size, const char* file, int line) {
    void* ptr = calloc(nmemb, size);
    if (!ptr) return NULL;

    AllocationInfo* info = (AllocationInfo*)malloc(sizeof(AllocationInfo));
    if (!info) {
        free(ptr);
        return NULL;
    }

    info->ptr = ptr;
    info->size = nmemb * size;
    info->file = strdup(file);
    if (!info->file) {
        free(ptr);
        free(info);
        return NULL;
    }
    info->line = line;
    info->is_freed = 0;
    info->next = head;
    head = info;

    printf(COLOR_GREEN "[ALLOC] %zu bytes (calloc) → %p (at %s:%d)\n" COLOR_RESET,
           nmemb * size, ptr, file, line);
           total_allocations++;
current_bytes += nmemb * size;
if (current_bytes > peak_bytes) peak_bytes = current_bytes;



    return ptr;
}

void* my_realloc(void* ptr, size_t new_size, const char* file, int line) {
    if (!ptr) {
        // realloc with NULL ptr behaves like malloc
        return my_malloc(new_size, file, line);
    }

    AllocationInfo* current = head;
    while (current) {
        if (current->ptr == ptr) {
            // Adjust current_bytes before realloc
            current_bytes -= current->size;

            void* new_ptr = realloc(ptr, new_size);
            if (!new_ptr) {
                // realloc failed, revert current_bytes change
                current_bytes += current->size;
                return NULL;
            }

            // Update tracking info
            current->ptr = new_ptr;
            current->size = new_size;

            free(current->file);  // ✅ Free the previous strdup'd file name
            current->file = strdup(file);
            current->line = line;

            current_bytes += new_size;
            if (current_bytes > peak_bytes) peak_bytes = current_bytes;

            printf(COLOR_GREEN "[REALLOC] %p resized to %zu bytes (at %s:%d)\n" COLOR_RESET,
                   new_ptr, new_size, file, line);

            return new_ptr;
        }
        current = current->next;
    }

    // Pointer not found in tracking list
    printf(COLOR_RED "[REALLOC] Warning: Reallocating untracked pointer %p (at %s:%d)\n" COLOR_RESET,
           ptr, file, line);

    return realloc(ptr, new_size);  // untracked realloc fallback
}

void* my_new(size_t size, const char* file, int line) {
    void* ptr = ::operator new(size, std::nothrow);
    if (!ptr) return nullptr;

    AllocationInfo* info = (AllocationInfo*)malloc(sizeof(AllocationInfo));
    if (!info) {
        ::operator delete(ptr);
        return nullptr;
    }

    info->ptr = ptr;
    info->size = size;
    info->file = strdup(file);
    if (!info->file) {
        ::operator delete(ptr);
        free(info);
        return nullptr;
    }

    info->line = line;
    info->is_freed = 0;
    info->next = head;
    head = info;

    current_bytes += size;  // ✅ Properly track allocation size
    if (current_bytes > peak_bytes) peak_bytes = current_bytes;
    total_allocations++;

    printf(COLOR_GREEN "[ALLOC] %zu bytes (new) → %p (at %s:%d)\n" COLOR_RESET, size, ptr, file, line);
    return ptr;
}

void my_delete(void* ptr, const char* file, int line) {
    if (!ptr) {
        printf(COLOR_YELLOW "[DELETE] Warning: Attempt to delete nullptr (at %s:%d)\n" COLOR_RESET, file, line);
        return;
    }

    AllocationInfo* current = head;
    while (current) {
        if (current->ptr == ptr) {
            if (current->is_freed) {
                const char* deletedAtFile = current->free_file ? current->free_file : "unknown";
                int deletedAtLine = current->free_line ? current->free_line : 0;
                const char* attemptFile = (file && strcmp(file, "unknown") != 0) ? file : deletedAtFile;
                int attemptLine = (line != 0) ? line : deletedAtLine;

                printf(COLOR_RED "[DELETE] Warning: Double delete detected on %p (originally deleted at %s:%d) (attempted again at %s:%d)\n" COLOR_RESET,
                       ptr,
                       deletedAtFile,
                       deletedAtLine,
                       attemptFile,
                       attemptLine);
                return;
            }

            printf(COLOR_YELLOW "[DELETE] %p deleted (originally allocated at %s:%d) (deleted at %s:%d)\n" COLOR_RESET,
                   ptr, current->file, current->line, file, line);

            current->is_freed = true;
            if (current->free_file) free(current->free_file);
            current->free_file = strdup(file);
            current->free_line = line;

            if (current_bytes >= current->size)
                current_bytes -= current->size;
            else
                current_bytes = 0;

            total_frees++;

            ::operator delete(ptr);
            return;
        }
        current = current->next;
    }

    printf(COLOR_RED "[DELETE] Warning: Deleting untracked pointer %p (at %s:%d)\n" COLOR_RESET, ptr, file, line);
}


void check_pointer(void* ptr, const char* file, int line) {
    if (!ptr) {
        printf(COLOR_YELLOW "[CHECK_PTR] Warning: Checking NULL pointer (at %s:%d)\n" COLOR_RESET, file, line);
        return;
    }

    AllocationInfo* current = head;
    while (current) {
        if (current->ptr == ptr) {
            if (current->is_freed) {
                printf(COLOR_RED "[CHECK_PTR] Warning: Use-after-free detected on %p (originally allocated at %s:%d) (checked at %s:%d)\n" COLOR_RESET,
                       ptr, current->file, current->line, file, line);
            }
            return;
        }
        current = current->next;
    }
    // Pointer not tracked at all
    printf(COLOR_YELLOW "[CHECK_PTR] Warning: Pointer %p not tracked (checked at %s:%d)\n" COLOR_RESET, ptr, file, line);
}

void* my_new_array(size_t size, const char* file, int line) {
    void* ptr = ::operator new[](size, std::nothrow);  // Use nothrow to avoid exceptions
    if (!ptr) return nullptr;

    AllocationInfo* info = (AllocationInfo*)malloc(sizeof(AllocationInfo));
    if (!info) {
        ::operator delete[](ptr);
        return nullptr;
    }

    info->ptr = ptr;
    info->size = size;

    info->file = strdup(file);
    if (!info->file) {
        free(info);
        ::operator delete[](ptr);
        return nullptr;
    }

    info->line = line;
    info->is_freed = false;
    info->is_array = true;  // Important for distinguishing delete vs delete[]
    info->next = head;
    head = info;

    total_allocations++;
    current_bytes += size;
    if (current_bytes > peak_bytes) {
        peak_bytes = current_bytes;
    }

    printf(COLOR_GREEN "[NEW ARRAY] %zu bytes → %p (at %s:%d)\n" COLOR_RESET, size, ptr, file, line);
    return ptr;
}

void my_delete_array(void* ptr, const char* file, int line) {
    if (!ptr) {
        printf(COLOR_YELLOW "[DELETE ARRAY] Warning: Attempt to delete[] nullptr (at %s:%d)\n" COLOR_RESET, file, line);
        return;
    }

    AllocationInfo* current = head;
    while (current) {
        if (current->ptr == ptr) {
            if (current->is_freed) {
                // If the current delete site is unknown, print the saved free location
                const char* freeSiteFile = current->free_file ? current->free_file : "unknown";
                int freeSiteLine = current->free_line ? current->free_line : 0;
                const char* attemptFile = (file && strcmp(file, "unknown") != 0) ? file : freeSiteFile;
                int attemptLine = (line != 0) ? line : freeSiteLine;

                printf(COLOR_RED "[DELETE ARRAY] Warning: Double delete[] detected on %p (originally deleted[] at %s:%d) (attempted again at %s:%d)\n" COLOR_RESET,
                ptr,
                freeSiteFile,
                freeSiteLine,
                attemptFile,
                attemptLine);

                return;
            }

            if (!current->is_array) {
                printf(COLOR_RED "[DELETE ARRAY] Warning: Deleting pointer with delete[] that was not allocated with new[] (at %s:%d)\n" COLOR_RESET, file, line);
                // optionally continue or return here
            }

            printf(COLOR_YELLOW "[DELETE ARRAY] %p deleted[] (originally allocated at %s:%d) (deleted at %s:%d)\n" COLOR_RESET,
                   ptr, current->file, current->line, file, line);

            // Mark freed and store free location
            current->is_freed = true;
            if (current->free_file) free(current->free_file);
            current->free_file = strdup(file);
            current->free_line = line;

            current_bytes -= current->size;
            total_frees++;

            ::operator delete[](ptr);
            return;
        }
        current = current->next;
    }

    // Pointer not found
    printf(COLOR_RED "[DELETE ARRAY] Warning: Deleting untracked pointer %p (at %s:%d)\n" COLOR_RESET, ptr, file, line);
}




void print_memory_report(void) {
    AllocationInfo* current = head;
    AllocationInfo* temp = NULL;
    int leaks = 0;

    printf("\n================ Memory Leak Report ================\n");

    while (current) {
        if (!current->is_freed) {
            printf(COLOR_RED "Leak: %zu bytes at %p (allocated at %s:%d)\n" COLOR_RESET,
                   current->size, current->ptr, current->file, current->line);
            leaks++;
        }

        temp = current;
        current = current->next;
        if (temp->file) free(temp->file);
        free(temp);
    }

    head = NULL;

    if (leaks == 0) {
        printf("No memory leaks detected.\n");
    } else {
        printf("Total leaks detected: %d\n", leaks);
    }

    printf("====================================================\n");
}
void print_memory_stats(void) {
    printf("\n========== Memory Usage Statistics ==========\n");
    printf("Total allocations: %zu\n", total_allocations);
    printf("Total frees: %zu\n", total_frees);
    printf("Current allocated bytes: %zu\n", current_bytes);
    printf("Peak allocated bytes: %zu\n", peak_bytes);
    printf("=============================================\n");
}

void* operator new(std::size_t size, const char* file, int line) {
    return my_new(size, file, line);
}
// Unsized delete operator - called when size info is not available
void operator delete(void* ptr) noexcept {
    (void)ptr;
}

// Sized delete operator - called when size info is passed
void operator delete(void* ptr, std::size_t size) noexcept {
    (void)size;  // mark unused to avoid warnings
    my_delete(ptr, "unknown", 0);
}

// Array new operator overload with file and line info
void* operator new[](std::size_t size, const char* file, int line) {
    return my_new_array(size, file, line);
}

// Unsized array delete operator
void operator delete[](void* ptr) noexcept {
    (void)ptr;
}

// Sized array delete operator
void operator delete[](void* ptr, std::size_t size) noexcept {
    (void)size;  // unused
    my_delete_array(ptr, "unknown", 0);
}


