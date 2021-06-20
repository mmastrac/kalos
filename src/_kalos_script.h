#pragma once

/** @file
 * Utilities for introspection of compiled script files.
 */

#include <stdbool.h>
#include <stdint.h>

#define KALOS_OP(x, in, out, args) KALOS_OP_##x,
typedef enum kalos_op {
#include "kalos_constants.inc"
    KALOS_OP_MAX,
} kalos_op;

extern const char* kalos_op_strings[KALOS_OP_MAX + 1];

typedef kalos_buffer kalos_script;

#pragma pack(push, 1)
typedef struct kalos_script_header {
    uint8_t signature[4];
    uint16_t globals_size;
    uint16_t length;
} kalos_script_header;

typedef struct kalos_section_header {
    kalos_export_address handle_address;
    uint16_t locals_size;
    kalos_int next_section;
} kalos_section_header;
#pragma pack(pop)

void kalos_dump(kalos_script* script, char* buffer);
typedef bool (*kalos_walk_fn)(void* context, kalos_script* script, kalos_section_header* header, uint16_t pc, uint16_t length);
void kalos_walk(kalos_script* script, void* context, kalos_walk_fn fn);
uint16_t kalos_find_section(kalos_script* script, kalos_export_address handle_address, kalos_section_header** header);
