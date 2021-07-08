#pragma once
#include "../kalos_parse.h"

const char* compiler_script();
kalos_module_parsed compiler_idl(kalos_state* state);
kalos_buffer compiler_idl_script(kalos_state* state);
