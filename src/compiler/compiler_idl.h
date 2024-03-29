#pragma once
#include "../kalos_run.h"

kalos_module_parsed kalos_idl_parse_module(const char* s, kalos_state* state);
bool kalos_idl_generate_dispatch(kalos_module_parsed, kalos_state* state);

bool kalos_module_idl_dynamic_dispatch(kalos_run_state* state, const char* module, const char* name,
    int param_count, kalos_stack* stack, bool retval);
void kalos_module_idl_sys_trigger_main(kalos_run_state* state, kalos_object_ref* args);

kalos_string kalos_compiler_read_file(kalos_state* state, const char* base, const char* ext);
kalos_loaded_script kalos_entrypoint_loader(kalos_state* state, const char* base, kalos_load_result* result);
void kalos_entrypoint_unloader(kalos_state* state, kalos_loaded_script script);

kalos_object_ref kalos_compiler_compile_idl(kalos_state* state, kalos_string* idl);
void kalos_compiler_generate_idl(kalos_state* state, kalos_object_ref* modules);
kalos_buffer kalos_compiler_idl_script(kalos_state* state);
kalos_object_ref kalos_compiler_compile_script(kalos_state* state, kalos_object_ref* modules, kalos_string* script);
void kalos_compiler_run_script(kalos_state* state, kalos_object_ref* script, kalos_object_ref* args);
kalos_string kalos_compiler_get_buffer(kalos_state* state, kalos_object_ref* buffer);
kalos_object_ref kalos_get_compiler_module(kalos_state* state);
void kalos_idl_set_module(kalos_state* state, kalos_object_ref* object);

kalos_object_ref kalos_compiler_lex_script(kalos_state* state, kalos_string* script);
void kalos_lexer_free(kalos_state* state, kalos_object_ref* object);
kalos_object_ref kalos_lexer_peek_token(kalos_state* state, kalos_object_ref* object);
kalos_object_ref kalos_lexer_read_token(kalos_state* state, kalos_object_ref* object);
void kalos_lexer_token_read_string_free(kalos_state* state, kalos_object_ref* object);
kalos_string kalos_lexer_token_read_string(kalos_state* state, kalos_object_ref* object);
kalos_string kalos_lexer_token_read_token(kalos_state* state, kalos_object_ref* object);
bool kalos_lexer_token_read_is_word(kalos_state* state, kalos_object_ref* object);
