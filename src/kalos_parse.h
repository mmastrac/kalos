#pragma once

#include "kalos.h"
#include "kalos_module.h"

#define KALOS_OP(x, in, out) KALOS_OP_##x,
typedef enum kalos_op {
#include "kalos_constants.inc"
    KALOS_OP_MAX,
} kalos_op;

extern const char* kalos_op_strings[KALOS_OP_MAX + 1];

typedef struct kalos_script {
    uint8_t* script_ops;
    uint16_t script_buffer_size;
} kalos_script;

typedef struct kalos_parse_result {
    bool success;
    const char* error;
    int line;
} kalos_parse_result;

kalos_parse_result kalos_parse(const char kalos_far* script_text, kalos_module_parsed modules, kalos_script* script);
