#pragma once
#include "../kalos_run.h"

kalos_module_parsed kalos_idl_parse_module(const char* s, kalos_state* state);
bool kalos_idl_generate_dispatch(kalos_module_parsed, kalos_state* state);

bool kalos_module_idl_dynamic_dispatch(kalos_run_state* state, const char* module, const char* name,
    int param_count, kalos_stack* stack, bool retval);
void kalos_module_idl_sys_trigger_main(kalos_run_state* state, kalos_object_ref* args);
