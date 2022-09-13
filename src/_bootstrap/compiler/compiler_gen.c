#include "compiler_gen.h"
#include "compiler_idl.h"

const char* compiler_script_text() {
    return 
        #include "compiler_idl.kalos.inc"
    ;
}

const char* compiler_idl_script_text() {
    return 
        #include "compiler.kidl.inc"
    ;
}

kalos_module_parsed compiler_idl(kalos_state* state) {
    return kalos_idl_parse_module(compiler_idl_script_text(), state);
}

kalos_buffer compiler_idl_script(kalos_state* state) {
    kalos_parse_options options = {0};
    kalos_buffer script = {0};
    kalos_module_parsed modules = compiler_idl(state);
    kalos_parse_result result = kalos_parse_buffer(compiler_script_text(), modules, options, state, &script);
    kalos_buffer_free(modules);
    if (result.error) {
        if (state->error) {
            state->error(state->context, result.line, result.error);
        }
    }
    return script;
}
