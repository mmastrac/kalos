#pragma once

/** @file
 * The Kalos parser for parsing script text into compiled binaries.
 */

#include "kalos.h"
#include "_kalos_module.h"
#include "_kalos_script.h"

typedef struct kalos_parse_result {
    bool success;
    const char* error;
    int line;
} kalos_parse_result;

typedef struct kalos_parse_options {
    kalos_int flags;
} kalos_parse_options;

/**
 * Parse a script using the given modules and options.
 */
kalos_parse_result kalos_parse(const char kalos_far* script_text, kalos_module_parsed modules, kalos_parse_options options, kalos_script* script);
