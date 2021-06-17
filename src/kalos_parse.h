#pragma once

#include "kalos.h"
#include "kalos_module.h"

#define KALOS_OP(x, in, out, args) KALOS_OP_##x,
typedef enum kalos_op {
#include "kalos_constants.inc"
    KALOS_OP_MAX,
} kalos_op;

extern const char* kalos_op_strings[KALOS_OP_MAX + 1];

typedef struct kalos_script {
    uint8_t* script_ops;
    uint16_t script_buffer_size;
} kalos_script;

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

typedef struct kalos_parse_result {
    bool success;
    const char* error;
    int line;
} kalos_parse_result;

typedef struct kalos_parse_options {
    bool robust_dispatch;
} kalos_parse_options;

kalos_parse_result kalos_parse(const char kalos_far* script_text, kalos_module_parsed modules, kalos_parse_options options, kalos_script* script);
