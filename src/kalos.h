#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __WATCOMC__
#define kalos_far __far
#else
#define kalos_far 
#endif

/**
 * The basic integer type of Kalos.
 */
typedef int16_t kalos_int;

typedef struct {
    uint16_t count;
    const char* s;
} kalos_string_allocated;

/**
 * The basic string type of Kalos.
 */
typedef struct { 
    kalos_int length__;
    union {
        kalos_string_allocated* sa;
        const char* sc;
    };
} kalos_string;

/**
 * A writable string type for Kalos that must be committed before use.
 */
typedef struct { kalos_string_allocated* s; } kalos_writable_string;

typedef void* (*kalos_mem_alloc_fn)(size_t);
typedef void* (*kalos_mem_realloc_fn)(void*, size_t);
typedef void (*kalos_mem_free_fn)(void*);
typedef void (*kalos_print_fn)(const char*);
typedef void (*kalos_error_fn)(int line, const char* error);

/**
 * The basic environment required for all Kalos operations. Some functions may
 * be passed in as NULL in certain cases.
 */
typedef struct {
    kalos_mem_alloc_fn alloc;
    kalos_mem_realloc_fn realloc;
    kalos_mem_free_fn free;
    kalos_print_fn print;
    kalos_error_fn error;
} kalos_basic_environment;

typedef void* kalos_state;

static inline void* kalos_mem_alloc(kalos_state state_, size_t size) {
    return ((kalos_basic_environment*)state_)->alloc(size);
}
static inline void* kalos_mem_realloc(kalos_state state_, void* ptr, size_t size) {
    return ((kalos_basic_environment*)state_)->realloc(ptr, size);
}
static inline void kalos_mem_free(kalos_state state_, void* ptr) {
    ((kalos_basic_environment*)state_)->free(ptr);
}

typedef struct kalos_buffer {
    uint8_t* buffer;
} kalos_buffer;

typedef kalos_buffer kalos_script;
typedef kalos_buffer kalos_module_parsed;

kalos_buffer kalos_buffer_alloc(kalos_basic_environment* env, size_t size);
size_t kalos_buffer_size(kalos_buffer buffer);
void kalos_buffer_free(kalos_buffer buffer);
