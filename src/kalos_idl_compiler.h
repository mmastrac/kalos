#pragma once
#include "kalos_module.h"

kalos_module_parsed kalos_idl_parse_module(const char* s);
bool kalos_idl_generate_dispatch(kalos_module_parsed);
