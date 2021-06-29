// Standard guards for main entry headers (https://github.com/open-watcom/open-watcom-v2/issues/714)
#ifndef KALOS_H
#define KALOS_H

/** @file
 * Common include for all Kalos users.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __WATCOMC__
#define kalos_far __far
#define KALOS_MAYBE_UNUSED_BEGIN _Pragma("off (unreferenced)")
#define KALOS_MAYBE_UNUSED_END _Pragma("on (unreferenced)")
#define KALOS_ALLOW_UNREACHABLE_CODE_BEGIN _Pragma("warning 201 9")
#define KALOS_ALLOW_UNREACHABLE_CODE_END _Pragma("warning 201 3")
#elif __AVR__
#define kalos_far __flash
#else
#define kalos_far 
#define KALOS_MAYBE_UNUSED_BEGIN
#define KALOS_MAYBE_UNUSED_END
#define KALOS_ALLOW_UNREACHABLE_CODE_BEGIN
#define KALOS_ALLOW_UNREACHABLE_CODE_END
#endif

/**
 * The basic integer type of Kalos.
 */
typedef int16_t kalos_int;
#define KALOS_INT_MAX ((int16_t)(0x7fff))
#define KALOS_INT_MIN ((int16_t)(0x8000))

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
typedef void (*kalos_print_fn)(void* context, const char*);
typedef void (*kalos_error_fn)(void* context, int line, const char* error);

#define KALOS_BASIC_ENVIRONMENT                                               \
    /**                                                                       \
     * Must be set to zero by user code. Used to identify the structure.      \
     */                                                                       \
    uint8_t reserved;                                                         \
    /**                                                                       \
     * User-defined context.                                                  \
     */                                                                       \
    void* context;                                                            \
    kalos_mem_alloc_fn alloc;                                                 \
    kalos_mem_realloc_fn realloc;                                             \
    kalos_mem_free_fn free;                                                   \
    kalos_print_fn print;                                                     \
    kalos_error_fn error                                                      \

/**
 * The basic environment required for all Kalos operations. Some functions may
 * be passed in as NULL in certain cases.
 */
typedef struct {
    KALOS_BASIC_ENVIRONMENT;
} kalos_state;

/**
 * Helper function to call the allocation method from a kalos_state*.
 */
static inline void* kalos_mem_alloc(kalos_state* state, size_t size) {
    return state->alloc(size);
}
/**
 * Helper function to call the re-allocation method from a kalos_state*.
 */
static inline void* kalos_mem_realloc(kalos_state* state, void* ptr, size_t size) {
    return state->realloc(ptr, size);
}
/**
 * Helper function to call the allocation-freeing method from a kalos_state*.
 */
static inline void kalos_mem_free(kalos_state* state, void* ptr) {
    state->free(ptr);
}

/**
 * A sized buffer that is free-able without having access to the kalos_state* it was created under.
 */
typedef struct kalos_buffer {
    uint8_t* buffer;
} kalos_buffer;

/**
 * Allocate a buffer that can be freed without reference to the Kalos environment.
 */
kalos_buffer kalos_buffer_alloc(kalos_state* state, size_t size);

/**
 * Get the intrinsic size of a Kalos buffer.
 */
size_t kalos_buffer_size(kalos_buffer buffer);

/**
 * Resize a buffer's intrinsic size.
 */
void kalos_buffer_resize(kalos_buffer* buffer, size_t size);

/**
 * Free a Kalos buffer.
 */
void kalos_buffer_free(kalos_buffer buffer);

#endif
