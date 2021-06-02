#pragma once
#include "kalos.h"

typedef struct kalos_idl_callbacks {
    void (*prefix)(void* context, const char* prefix);

    void (*begin_module)(void* context, const char* module);
    void (*end_module)(void* context);

    void (*begin_function)(void* context, const char* name);
    void (*function_arg)(void* context, const char* name, const char* type, bool is_varargs);
    void (*end_function)(void* context, const char* name, const char* type, const char* symbol);

    void (*constant_string)(void* context, const char* name, const char* type, const char* string);
    void (*constant_number)(void* context, const char* name, const char* type, kalos_int number);

    void (*property)(void* context, const char* name, const char* type, const char* mode, const char* symbol, const char* symbol2);
} kalos_idl_callbacks;

bool kalos_idl_parse_callback(const char* s, void* context, kalos_idl_callbacks* callbacks);
