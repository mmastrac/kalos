#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __WATCOMC__
#define kalos_far __far
#else
#define kalos_far 
#endif

typedef int16_t kalos_int;
typedef struct { uint16_t count; uint16_t length; const char* s; } kalos_string;
typedef struct { char* s; } kalos_writable_string;

typedef void* (*kalos_mem_alloc_fn)(size_t);
typedef void (*kalos_mem_free_fn)(void*);
typedef void* kalos_state;
static inline void* kalos_mem_alloc(kalos_state state_, size_t size) {
    struct kalos_state_public {
        kalos_mem_alloc_fn alloc;
    }* state = state_;
    return state->alloc(size);
}
