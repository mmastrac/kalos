#pragma once

/** @file
 * Utilities for introspection of compiled script files.
 */

#include "kalos.h"
#include "_kalos_module.h"

#define KALOS_OP(x, args) KALOS_OP_##x,
typedef enum kalos_op {
#include "_kalos_constants.inc"
    KALOS_OP_MAX,
} kalos_op;

extern const char* kalos_op_strings[KALOS_OP_MAX + 1];

typedef uint8_t kalos_far* kalos_script;
typedef const uint8_t kalos_far* const_kalos_script;

#pragma pack(push, 1)
typedef struct kalos_script_header {
    uint8_t signature[4];
    uint16_t globals_size;
    uint16_t length;
} kalos_script_header;

typedef struct kalos_section_header {
    kalos_export_address handler_address;
    uint16_t locals_size;
    kalos_int next_section;
} kalos_section_header;
#pragma pack(pop)

void kalos_dump(const_kalos_script script, char* buffer);
typedef bool (*kalos_walk_fn)(void* context, const_kalos_script script, const kalos_section_header kalos_far* header, uint16_t pc, uint16_t length);
void kalos_walk(const_kalos_script script, void* context, kalos_walk_fn fn);
uint16_t kalos_find_section(const_kalos_script script, kalos_export_address handler_address, const kalos_section_header kalos_far** header);
kalos_module_header* kalos_find_idl(const_kalos_script script);
