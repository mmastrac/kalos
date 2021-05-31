#pragma once
#include "kalos_module.h"

typedef struct kalos_module_parsed {
    void* data;
    size_t size;
} kalos_module_parsed;

kalos_module_parsed kalos_idl_parse_module(const char* s);
kalos_module** kalos_idl_unpack_module(uint8_t* data);
