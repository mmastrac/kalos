#pragma once
#include "_kalos_module.h"

kalos_module_parsed kalos_idl_parse_module(const char* s, kalos_state* state);
bool kalos_idl_generate_dispatch(kalos_module_parsed, kalos_state* state);

kalos_object_ref kalos_file_open(kalos_state* state, kalos_string* file_, kalos_int mode_);
kalos_string kalos_file_readline(kalos_state* state, kalos_object_ref* object);
kalos_string kalos_file_read_all(kalos_state* state, kalos_object_ref* object);
kalos_string kalos_file_read(kalos_state* state, kalos_object_ref* object, kalos_int size);
void kalos_file_close(kalos_state* state, kalos_object_ref* object);
