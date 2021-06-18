#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __WATCOMC__
#define kalos_far __far
#else
#define kalos_far 
#endif

typedef int16_t kalos_int;

typedef struct {
    uint16_t count;
    const char* s;
} kalos_string_allocated;
typedef struct { 
    kalos_int length__;
    union {
        kalos_string_allocated* sa;
        const char* sc;
    };
} kalos_string;
typedef struct { kalos_string_allocated* s; } kalos_writable_string;

typedef struct kalos_script {
    uint8_t* script_ops;
    uint16_t script_buffer_size;
} kalos_script;

typedef void* (*kalos_mem_alloc_fn)(size_t);
typedef void* (*kalos_mem_realloc_fn)(void*, size_t);
typedef void (*kalos_mem_free_fn)(void*);
typedef void* kalos_state;
static inline void* kalos_mem_alloc(kalos_state state_, size_t size) {
    struct kalos_state_public {
        kalos_mem_alloc_fn alloc;
        kalos_mem_realloc_fn realloc;
        kalos_mem_free_fn free;
    }* state = state_;
    return state->alloc(size);
}
static inline void* kalos_mem_realloc(kalos_state state_, void* ptr, size_t size) {
    struct kalos_state_public {
        kalos_mem_alloc_fn alloc;
        kalos_mem_realloc_fn realloc;
        kalos_mem_free_fn free;
    }* state = state_;
    return state->realloc(ptr, size);
}
static inline void kalos_mem_free(kalos_state state_, void* ptr) {
    struct kalos_state_public {
        kalos_mem_alloc_fn alloc;
        kalos_mem_realloc_fn realloc;
        kalos_mem_free_fn free;
    }* state = state_;
    state->free(ptr);
}
