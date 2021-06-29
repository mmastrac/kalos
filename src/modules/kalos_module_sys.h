#pragma once
#include "../kalos_run.h"

void kalos_sys_set_args(int argc, char* argv[]);
kalos_object_ref kalos_sys_get_args(kalos_run_state* state);
kalos_string kalos_sys_get_env(kalos_state* state, kalos_string* name);
