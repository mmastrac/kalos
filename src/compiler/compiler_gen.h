#pragma once
#include "../kalos_parse.h"

static const char* compiler_script_text_inc() {
    return 
        #include "compiler.kalos.inc"
    ;
}

static const char* compiler_idl_script_text_inc() {
    return 
        #include "compiler_idl.kalos.inc"
    ;
}

static const char* compiler_kidl_text_inc() {
    return 
        #include "compiler.kidl.inc"
    ;
}

kalos_module_parsed compiler_idl(kalos_state* state);
kalos_buffer compiler_idl_script(kalos_state* state);
