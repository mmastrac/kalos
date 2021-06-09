#pragma once
#include "kalos.h"

typedef struct kalos_idl_callbacks {
    void (*error)(void* context, const char* start, uint16_t error_offset);
    bool (*prefix)(void* context, const char* prefix);

    void (*begin_module)(void* context, const char* module);
    void (*end_module)(void* context);

    void (*begin_object)(void* context, const char* object);
    void (*end_object)(void* context);

    void (*begin_function)(void* context, const char* name);
    void (*function_arg)(void* context, const char* name, const char* type, bool is_varargs);
    void (*end_function)(void* context, const char* name, const char* type, const char* symbol);

    void (*begin_handle)(void* context, const char* name);
    void (*handle_arg)(void* context, const char* name, const char* type, bool is_varargs);
    void (*end_handle)(void* context);

    bool (*constant_string)(void* context, const char* name, const char* type, const char* string);
    void (*constant_number)(void* context, const char* name, const char* type, kalos_int number);

    void (*property)(void* context, const char* name, const char* type, const char* mode, const char* symbol, const char* symbol2);
} kalos_idl_callbacks;

bool kalos_idl_parse_callback(const char* s, void* context, kalos_idl_callbacks* callbacks);
