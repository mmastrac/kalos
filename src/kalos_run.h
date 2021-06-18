#pragma once

#include <stddef.h>
#include <stdint.h>
#include "kalos.h"
#include "kalos_module.h"

kalos_state kalos_init_for_test(kalos_basic_environment* env);
kalos_state kalos_init(kalos_script* script, kalos_dispatch* dispatch, kalos_basic_environment* env);
void kalos_load_arg_any(kalos_state state_, kalos_int index, kalos_value* arg);
void kalos_load_arg_object(kalos_state state, kalos_int index, kalos_object_ref* arg);
void kalos_load_arg_number(kalos_state state, kalos_int index, kalos_int arg);
void kalos_load_arg_string(kalos_state state, kalos_int index, kalos_string* arg);
void kalos_load_arg_bool(kalos_state state, kalos_int index, bool arg);
void kalos_trigger(kalos_state state, kalos_export_address handle_address);
void kalos_run_free(kalos_state state);
