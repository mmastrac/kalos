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
void kalos_load_arg_object(kalos_state state, kalos_int index, kalos_object* arg);
void kalos_load_arg_number(kalos_state state, kalos_int index, kalos_int arg);
void kalos_load_arg_string(kalos_state state, kalos_int index, kalos_string arg);
void kalos_load_arg_bool(kalos_state state, kalos_int index, bool arg);
void kalos_trigger(kalos_state state, kalos_export_address handle_address);
void kalos_run_free(kalos_state state);

kalos_object* kalos_allocate_object(kalos_state state, size_t context_size);

typedef void (*kalos_iterable_fn)(kalos_state state, void* context, uint16_t index, kalos_value* output);
kalos_object* kalos_allocate_sized_iterable(kalos_state state, kalos_iterable_fn fn, size_t context_size, void** context, uint16_t count);
kalos_object* kalos_allocate_prop_object(kalos_state state, void* context, kalos_object_dispatch dispatch);
