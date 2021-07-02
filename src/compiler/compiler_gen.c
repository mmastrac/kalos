#include "compiler_gen.h"
#include "compiler_idl.h"

kalos_module_parsed compiler_idl(kalos_state* state) {
    const char IDL_COMPILER_IDL[] = {
        #include "compiler.kidl.inc"
    };
    return kalos_idl_parse_module(IDL_COMPILER_IDL, state);
}

kalos_buffer compiler_idl_script(kalos_state* state) {
    const char IDL_COMPILER_SCRIPT[] = {
        #include "compiler_idl.kalos.inc"
    };
    kalos_parse_options options = {0};
    kalos_buffer script = {0};
    kalos_module_parsed modules = compiler_idl(state);
    kalos_parse_result result = kalos_parse_buffer(IDL_COMPILER_SCRIPT, modules, options, state, &script);
    kalos_buffer_free(modules);
    if (result.error) {
        if (state->error) {
            state->error(state->context, result.line, result.error);
        }
    }
    return script;
}
