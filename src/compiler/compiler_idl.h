#pragma once
#include "../kalos_run.h"

kalos_module_parsed kalos_idl_parse_module(const char* s, kalos_state* state);
bool kalos_idl_generate_dispatch(kalos_module_parsed, kalos_state* state);

bool kalos_module_idl_dynamic_dispatch(kalos_run_state* state, const char* module, const char* name,
    int param_count, kalos_stack* stack, bool retval);
void kalos_module_idl_sys_trigger_main(kalos_run_state* state, kalos_object_ref* args);

kalos_object_ref kalos_compiler_compile_idl(kalos_state* state, kalos_string* idl);
kalos_object_ref kalos_compiler_compile_script(kalos_state* state, kalos_object_ref* modules, kalos_string* script);
void kalos_compiler_run_script(kalos_state* state, kalos_object_ref* script, kalos_object_ref* args);
kalos_string kalos_compiler_get_buffer(kalos_state* state, kalos_object_ref* buffer);
kalos_string kalos_compiler_get_idl_script(kalos_state* state);
kalos_string kalos_compiler_get_compiler_idl(kalos_state* state);
