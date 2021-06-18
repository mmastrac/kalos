#pragma once
#include "kalos_module.h"

kalos_module_parsed kalos_idl_parse_module(const char* s, kalos_basic_environment* env);
void kalos_idl_free_module(kalos_module_parsed parsed_modules, kalos_basic_environment* env);
bool kalos_idl_generate_dispatch(kalos_module_parsed, kalos_basic_environment* env);
