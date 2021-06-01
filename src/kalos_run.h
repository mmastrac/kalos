#pragma once

#include <stddef.h>
#include <stdint.h>
#include "kalos.h"

typedef void (*kalos_error_fn)(char* error);

typedef struct kalos_fn {
    kalos_mem_alloc_fn alloc;
    kalos_mem_free_fn free;
    kalos_error_fn error;
} kalos_fn;

typedef struct kalos_script kalos_script;
typedef struct kalos_module kalos_module;
kalos_state kalos_init_for_test(kalos_fn* fns);
kalos_state kalos_init(kalos_script* script, kalos_dispatch_fn* modules, kalos_fn* fns);
void kalos_trigger(kalos_state state, char* handler);
void kalos_run_free(kalos_state state);

kalos_object* kalos_allocate_object(kalos_state state, size_t context_size);
