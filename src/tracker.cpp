#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>  // for std::nothrow
#include "tracker.h"
#include <fstream>


// ANSI color codes
#define COLOR_RESET   "\x1b[0m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"



AllocationInfo* head = NULL;
size_t total_allocations = 0;
size_t total_frees = 0;
size_t current_bytes = 0;
size_t peak_bytes = 0;


void* my_malloc(size_t size, const char* file, int line) {
    void* ptr = malloc(size);
    if (!ptr) return NULL;

    AllocationInfo* info = new AllocationInfo();
    if (!info) {
        free(ptr);
        return NULL;
    }

    info->ptr = ptr;
    info->size = size;

    // Defensive copy of file name
    info->file = file ? file : "unknown";
    info->line = line;

    info->is_array = false;           // Important
    info->is_freed = false;
    info->free_file = "";           // Initialize to null
    info->free_line = -1;             // Use -1 or some invalid line marker

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
                // Mark as double free
                current->double_free = true;

                printf(COLOR_RED "[FREE] Warning: Double free detected on %p "
                       "(originally freed at %s:%d) (attempted again at %s:%d)\n" COLOR_RESET,
                       ptr,
                       current->free_file.empty() ? "unknown" : current->free_file.c_str(),
                       current->free_line,
                       file, line);
                return;
            }

            printf(COLOR_YELLOW "[FREE] %p freed (originally allocated at %s:%d) (freed at %s:%d)\n" COLOR_RESET,
                   ptr,
                   current->file.empty() ? "unknown" : current->file.c_str(),
                   current->line, file, line);

            current->is_freed = true;
            current_bytes -= current->size;
            total_frees++;

            // Save free site
            current->free_file = file ? file : "unknown";
            current->free_line = line;

            free(ptr);
            return;
        }
        current = current->next;
    }

    // Pointer not found in tracked allocations
    printf(COLOR_RED "[FREE] Warning: Attempt to free untracked pointer %p (at %s:%d)\n" COLOR_RESET, ptr, file, line);
}


void* my_calloc(size_t nmemb, size_t size, const char* file, int line) {
    void* ptr = calloc(nmemb, size);
    if (!ptr) return NULL;

    AllocationInfo* info = new AllocationInfo();
    if (!info) {
        free(ptr);
        return NULL;
    }

    info->ptr = ptr;
    info->size = nmemb * size;
    info->file = file ? file : "unknown"; // std::string assignment, no strdup
    info->line = line;
    info->is_array = false; // calloc is for non-arrays
    info->is_freed = false;
    info->free_file = "";   // empty string instead of NULL
    info->free_line = -1;
    info->next = head;
    head = info;

    printf(COLOR_GREEN "[ALLOC] %zu bytes (calloc) → %p (at %s:%d)\n" COLOR_RESET,
           nmemb * size, ptr, info->file.c_str(), line);

    total_allocations++;
    current_bytes += nmemb * size;
    if (current_bytes > peak_bytes) {
        peak_bytes = current_bytes;
    }

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
            if (current->is_freed) {
                printf(COLOR_RED "[REALLOC] Error: Attempt to realloc a freed pointer %p (originally freed at %s:%d)\n" COLOR_RESET,
                       ptr,
                       current->free_file.empty() ? "unknown" : current->free_file.c_str(),
                       current->free_line);
                return NULL;
            }

            // Adjust memory usage stats
            current_bytes -= current->size;

            void* new_ptr = realloc(ptr, new_size);
            if (!new_ptr) {
                // Failed to realloc, restore original stats
                current_bytes += current->size;
                return NULL;
            }

            // Update tracking info
            current->ptr = new_ptr;
            current->size = new_size;

            current->file = file ? file : "unknown";
            current->line = line;

            current_bytes += new_size;
            if (current_bytes > peak_bytes) peak_bytes = current_bytes;

            printf(COLOR_GREEN "[REALLOC] %p resized to %zu bytes (at %s:%d)\n" COLOR_RESET,
                   new_ptr, new_size, current->file.c_str(), line);

            return new_ptr;
        }
        current = current->next;
    }

    // Untracked pointer
    printf(COLOR_RED "[REALLOC] Warning: Reallocating untracked pointer %p (at %s:%d)\n" COLOR_RESET,
           ptr, file, line);

    return realloc(ptr, new_size); // Fallback: still return valid pointer, but untracked
}

void* my_new(size_t size, const char* file, int line) {
    void* ptr = ::operator new(size, std::nothrow);
    if (!ptr) return nullptr;

    AllocationInfo* info = new AllocationInfo();
    if (!info) {
        ::operator delete(ptr);
        return nullptr;
    }

    info->ptr = ptr;
    info->size = size;
    info->file = file ? file : "unknown";  // std::string handles copying
    info->line = line;
    info->is_array = false;  // new for single object
    info->is_freed = false;
    info->free_file = "";
    info->free_line = -1;
    info->next = head;
    head = info;

    current_bytes += size;
    if (current_bytes > peak_bytes) peak_bytes = current_bytes;
    total_allocations++;

    printf(COLOR_GREEN "[ALLOC] %zu bytes (new) → %p (at %s:%d)\n" COLOR_RESET,
           size, ptr, info->file.c_str(), line);

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
                current->double_free = true;
                printf(COLOR_RED "[DELETE] Warning: Double delete detected on %p "
                       "(originally deleted at %s:%d) (attempted again at %s:%d)\n" COLOR_RESET,
                       ptr,
                       current->free_file.empty() ? "unknown" : current->free_file.c_str(),
                       current->free_line > 0 ? current->free_line : 0,
                       file, line);
                return;
            }

            printf(COLOR_YELLOW "[DELETE] %p deleted (originally allocated at %s:%d) (deleted at %s:%d)\n" COLOR_RESET,
                   ptr,
                   current->file.empty() ? "unknown" : current->file.c_str(),
                   current->line,
                   file, line);

            current->is_freed = true;
            current->free_file = file ? file : "unknown";
            current->free_line = line;

            current_bytes = (current_bytes >= current->size) ? (current_bytes - current->size) : 0;
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
    current->dangling_pointer = true; 
                printf(COLOR_RED "[CHECK_PTR] Warning: Use-after-free detected on %p\n"
                       "           → originally allocated at %s:%d\n"
                       "           → freed at %s:%d\n"
                       "           → checked at %s:%d\n" COLOR_RESET,
                       ptr,
                       current->file.empty() ? "unknown" : current->file.c_str(), current->line,
                       current->free_file.empty() ? "unknown" : current->free_file.c_str(),
                       current->free_line >= 0 ? current->free_line : 0,
                       file, line);
            }
            return;  // Found and not freed: valid pointer
        }
        current = current->next;
    }

    // Not found in tracking list
    printf(COLOR_YELLOW "[CHECK_PTR] Warning: Pointer %p not tracked (checked at %s:%d)\n" COLOR_RESET, ptr, file, line);
    return;
}



void* my_new_array(size_t size, const char* file, int line) {
    void* ptr = ::operator new[](size, std::nothrow);  // Avoid throwing exceptions
    if (!ptr) return nullptr;

    AllocationInfo* info = new AllocationInfo();
    if (!info) {
        ::operator delete[](ptr);
        return nullptr;
    }

    info->ptr = ptr;
    info->size = size;
    info->file = file ? file : "unknown";  // std::string assignment
    info->line = line;
    info->is_freed = false;
    info->is_array = true;  // Important flag
    info->free_file = "";   // initialize as empty string
    info->free_line = -1;
    info->next = head;
    head = info;

    total_allocations++;
    current_bytes += size;
    if (current_bytes > peak_bytes) peak_bytes = current_bytes;

    printf(COLOR_GREEN "[NEW ARRAY] %zu bytes → %p (at %s:%d)\n" COLOR_RESET,
           size, ptr, info->file.c_str(), line);

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
                current->double_free = true;
                const char* freeSiteFile = current->free_file.empty() ? "unknown" : current->free_file.c_str();
                int freeSiteLine = (current->free_line >= 0) ? current->free_line : 0;
                const char* attemptFile = (file && strcmp(file, "unknown") != 0) ? file : freeSiteFile;
                int attemptLine = (line >= 0) ? line : freeSiteLine;

                printf(COLOR_RED "[DELETE ARRAY] Warning: Double delete[] detected on %p (originally deleted[] at %s:%d) (attempted again at %s:%d)\n" COLOR_RESET,
                       ptr,
                       freeSiteFile,
                       freeSiteLine,
                       attemptFile,
                       attemptLine);
                return;
            }

            if (!current->is_array) {
                printf(COLOR_RED "[DELETE ARRAY] Warning: Pointer %p deleted with delete[] but was NOT allocated with new[] (at %s:%d)\n" COLOR_RESET,
                       ptr, file, line);
                // Optional: you may choose to return here to prevent further delete[]
                // return;
            }

            printf(COLOR_YELLOW "[DELETE ARRAY] %p deleted[] (originally allocated at %s:%d) (deleted at %s:%d)\n" COLOR_RESET,
                   ptr,
                   current->file.empty() ? "unknown" : current->file.c_str(),
                   current->line,
                   file ? file : "unknown",
                   line);

            current->is_freed = true;
            current->free_file = file ? file : "unknown";
            current->free_line = line;

            current_bytes = (current_bytes >= current->size) ? (current_bytes - current->size) : 0;
            total_frees++;

            ::operator delete[](ptr);
            return;
        }
        current = current->next;
    }

    printf(COLOR_RED "[DELETE ARRAY] Warning: Deleting untracked pointer %p (at %s:%d)\n" COLOR_RESET, ptr, file, line);
}



void print_memory_report(void) {
    AllocationInfo* current = head;
    AllocationInfo* temp = nullptr;
    int leaks = 0;

    printf("\n================ Memory Leak Report ================\n");

    while (current) {
        if (!current->is_freed) {
            printf(COLOR_RED "Leak: %zu bytes at %p (allocated at %s:%d)\n" COLOR_RESET,
                   current->size, current->ptr, current->file.c_str(), current->line);
            leaks++;
        }

        temp = current;
        current = current->next;
        delete temp;  // Use delete if AllocationInfo was allocated via new
    }

    //head = nullptr;

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

std::string sanitize_string(const char* input) {
    if (!input) return "unknown";

    std::string s(input);
    std::string result;

    size_t i = 0;
    while (i < s.size()) {
        unsigned char c = s[i];

        if (c <= 0x7F) {
            // ASCII byte, valid single-byte UTF-8
            result += c;
            i++;
        } else if ((c & 0xE0) == 0xC0) {
            // 2-byte UTF-8 sequence
            if (i + 1 < s.size() &&
                (s[i+1] & 0xC0) == 0x80) {
                result += s[i];
                result += s[i+1];
                i += 2;
            } else {
                // Invalid sequence, replace
                result += '?';
                i++;
            }
        } else if ((c & 0xF0) == 0xE0) {
            // 3-byte UTF-8 sequence
            if (i + 2 < s.size() &&
                (s[i+1] & 0xC0) == 0x80 &&
                (s[i+2] & 0xC0) == 0x80) {
                result += s[i];
                result += s[i+1];
                result += s[i+2];
                i += 3;
            } else {
                result += '?';
                i++;
            }
        } else if ((c & 0xF8) == 0xF0) {
            // 4-byte UTF-8 sequence
            if (i + 3 < s.size() &&
                (s[i+1] & 0xC0) == 0x80 &&
                (s[i+2] & 0xC0) == 0x80 &&
                (s[i+3] & 0xC0) == 0x80) {
                result += s[i];
                result += s[i+1];
                result += s[i+2];
                result += s[i+3];
                i += 4;
            } else {
                result += '?';
                i++;
            }
        } else {
            // Invalid byte
            result += '?';
            i++;
        }
    }

    return result;
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

void debug_head_location() {
    printf("[CHECK] head address = %p (value = %p)\n", (void*)&head, (void*)head);
}


void export_json_report(const char* filename) {
    json report;
    report["total_allocations"] = total_allocations;
    report["total_frees"] = total_frees;
    report["current_bytes"] = current_bytes;
    report["peak_bytes"] = peak_bytes;
    report["allocations"] = json::array();

    if (!head) {
        printf("[WARN] Allocation list is empty! No data to export.\n");
    }

    AllocationInfo* current = head;
    size_t count = 0;

    while (current) {
        std::stringstream ss;
        ss << "0x" << std::hex << reinterpret_cast<uintptr_t>(current->ptr);

        json alloc_info = {
            {"address", ss.str()},
            {"size", current->size},
            {"file", current->file.empty() ? "unknown" : current->file},
            {"line", current->line},
            {"is_freed", current->is_freed},
            {"is_array", current->is_array},
            {"double_free", current->double_free},
            {"dangling_pointer", current->dangling_pointer}
        };

        if (current->is_freed) {
            alloc_info["free_file"] = current->free_file.empty() ? "unknown" : current->free_file;
            alloc_info["free_line"] = current->free_line;
        }

        report["allocations"].push_back(alloc_info);
        current = current->next;
        count++;
    }

    std::ofstream file(filename);
    if (file.is_open()) {
        file << report.dump(4);
        file.close();
        printf("[JSON] Report exported to %s (%zu records)\n", filename, count);
    } else {
        printf("[JSON] Failed to open file %s for writing\n", filename);
    }
}

void export_csv_report(const char* filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        printf("[CSV] Failed to open file %s for writing\n", filename);
        return;
    }

    // Header
    file << "Address,Size,File,Line,Freed,Free File,Free Line,Array?,Double Free,Dangling Ptr\n";

    AllocationInfo* current = head;
    while (current) {
        std::stringstream addr;
        addr << "0x" << std::hex << reinterpret_cast<uintptr_t>(current->ptr);

        file << addr.str() << ",";
        file << current->size << ",";
        file << (current->file.empty() ? "unknown" : current->file) << ",";
        file << current->line << ",";
        file << (current->is_freed ? "Yes" : "No") << ",";
        file << (current->free_file.empty() ? "unknown" : current->free_file) << ",";
        file << current->free_line << ",";
        file << (current->is_array ? "Yes" : "No") << ",";
        file << (current->double_free ? "Yes" : "No") << ",";
        file << (current->dangling_pointer ? "Yes" : "No") << "\n";

        current = current->next;
    }

    file.close();
    printf("[CSV] Report exported to %s\n", filename);
}
