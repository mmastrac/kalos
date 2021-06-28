#pragma once

/** @file
 * The Kalos parser for parsing script text into compiled binaries.
 */

#include "kalos.h"
#include "_kalos_module.h"
#include "_kalos_script.h"

/**
 * The result of a Kalos parse operation. If successful, success is true and error is NULL.
 */
typedef struct kalos_parse_result {
    /**
     * Was the parse operation successful?
     */
    bool success;
    /**
     * The error that occurred, or NULL if none.
     */
    const char* error;
    /**
     * The line on which the error occurred.
     */
    int line;
    /**
     * The size of the script.
     */
    size_t size;
} kalos_parse_result;

/**
 * Kalos parse options. Currently unused.
 */
typedef struct kalos_parse_options {
    /**
     * Kalos parse flags. Should be set to zero for future compatibility.
     */
    kalos_int flags;
} kalos_parse_options;

/**
 * Parse a script using the given modules and options. If the buffer is not large enough, this method will return the
 * required buffer size to continue.
 */
kalos_parse_result kalos_parse(const char kalos_far* script_text, kalos_module_parsed modules, kalos_parse_options options,
    kalos_script script, size_t script_size);

/**
 * Parse a script using the given modules and options, storing the result in a buffer.
 */
kalos_parse_result kalos_parse_buffer(const char kalos_far* script_text, kalos_module_parsed modules, kalos_parse_options options, kalos_state* state, kalos_buffer* buffer);
