#pragma once

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
