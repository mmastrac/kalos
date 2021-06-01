#pragma once

#include "kalos.h"
#include "kalos_stack.h"

#define MAX_KALOS_EXPORT_NAME_LENGTH 64
#define MAX_KALOS_ARGS 8

typedef enum kalos_function_type {
    FUNCTION_TYPE_VOID,
    FUNCTION_TYPE_BOOL,
    FUNCTION_TYPE_NUMBER,
    FUNCTION_TYPE_OBJECT,
    FUNCTION_TYPE_STRING,
    FUNCTION_TYPE_ANY,
    FUNCTION_TYPE_VARARGS,
} kalos_function_type;

typedef enum kalos_export_type {
    KALOS_EXPORT_TYPE_CONST_NUMBER,
    KALOS_EXPORT_TYPE_CONST_STRING,
    KALOS_EXPORT_TYPE_FUNCTION,
    KALOS_EXPORT_TYPE_PROP_READ,
    KALOS_EXPORT_TYPE_PROP_WRITE,
} kalos_export_type;

typedef struct kalos_arg {
    const char* name;
    kalos_function_type type;
} kalos_arg;

typedef struct kalos_function {
    kalos_arg args[MAX_KALOS_ARGS];
    uint8_t arg_count;
    kalos_function_type vararg_type;
    kalos_function_type return_type;
    const char* symbol;
} kalos_function;

typedef union kalos_export_entry {
    kalos_int const_number;
    const char* const_string;
    kalos_function function;
} kalos_export_entry;

typedef struct kalos_export {
    const char* name;
    kalos_export_type type;
    kalos_export_entry entry;
} kalos_export;

typedef void (*kalos_dispatch_fn)(kalos_state state, int function, kalos_stack* stack);

typedef struct kalos_module {
    const char* name;
    const char* prefix;
    kalos_export* exports;
    kalos_int export_count;
} kalos_module;
