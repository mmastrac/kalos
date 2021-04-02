#pragma once

#include <stddef.h>
#include <stdint.h>
#include "kalos.h"

typedef void* (*kalos_mem_alloc)(size_t);
typedef void (*kalos_mem_free)(void*);
typedef void (*kalos_error)(char* error);

typedef struct kalos_fn {
    kalos_mem_alloc alloc;
    kalos_mem_free free;
    kalos_error error;
} kalos_fn;

typedef struct kalos_script kalos_script;
typedef struct kalos_module kalos_module;
kalos_state kalos_init(kalos_script* script, kalos_module** modules, kalos_fn* fns);
void kalos_trigger(kalos_state state, char* handler);
void kalos_run_free(kalos_state state);

kalos_string kalos_allocate_string(kalos_state state, char* string);
kalos_string kalos_allocate_string_size(kalos_state state, int size);
