#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "kalos.h"

typedef void* (*kalos_mem_alloc)(size_t);

typedef enum kalos_string_format_align {
    KALOS_STRING_FORMAT_ALIGN_LEFT = '<',
    KALOS_STRING_FORMAT_ALIGN_RIGHT = '>',
    KALOS_STRING_FORMAT_ALIGN_PADDING_AFTER_SIGN = '=',
    KALOS_STRING_FORMAT_ALIGN_CENTER = '^',
} kalos_string_format_align;

typedef enum kalos_string_format_sign {
    KALOS_STRING_FORMAT_SIGN_BOTH = '+',
    KALOS_STRING_FORMAT_SIGN_NEGATIVE = '-',
    KALOS_STRING_FORMAT_SIGN_POSITIVE_SPACE = ' ',
} kalos_string_format_sign;

// The output buffer's format uses this struct for the string formatting token
typedef struct kalos_string_format {
    uint8_t fill; // Zero = space fill
    kalos_string_format_align align;
    kalos_string_format_sign sign;
    uint8_t alt_fmt;
    uint8_t min_width;
    uint8_t precision;
    uint8_t type;
} kalos_string_format;

bool kalos_parse_string_format(const char* s, kalos_string_format* string_format);
char* kalos_string_format_int(kalos_int value, kalos_string_format* string_format, kalos_mem_alloc alloc);
