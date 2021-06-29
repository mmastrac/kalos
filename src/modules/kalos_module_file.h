#pragma once
#include "../kalos_run.h"

kalos_object_ref kalos_file_open(kalos_state* state, kalos_string* file_, kalos_int mode_);
kalos_string kalos_file_readline(kalos_state* state, kalos_object_ref* object);
kalos_string kalos_file_read_all(kalos_state* state, kalos_object_ref* object);
kalos_string kalos_file_read(kalos_state* state, kalos_object_ref* object, kalos_int size);
void kalos_file_close(kalos_state* state, kalos_object_ref* object);
