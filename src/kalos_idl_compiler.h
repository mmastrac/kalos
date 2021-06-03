#pragma once
#include "kalos_module.h"

typedef void (*kalos_printer_fn)(const char* fmt, ...);

kalos_module_parsed kalos_idl_parse_module(const char* s);
bool kalos_idl_generate_dispatch(kalos_module_parsed, kalos_printer_fn);
