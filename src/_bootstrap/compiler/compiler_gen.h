#pragma once
#include "../kalos_parse.h"

const char* compiler_script_text();
const char* compiler_idl_script_text();
kalos_module_parsed compiler_idl(kalos_state* state);
kalos_buffer compiler_idl_script(kalos_state* state);
