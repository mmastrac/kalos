#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "kalos_string_format.h"
#include "../defines.h"

static inline const char* get_alt_format(kalos_string_format* format) {
    if (!format->alt_fmt) {
        return "";
    }
    switch (format->type) {
        case 'x': return "0x";
        case 'X': return "0X";
        case 'o': return "0o";
        default: return "";
    }
}

static inline char get_sign(kalos_string_format* format, kalos_int value) {
    if (value < 0) {
        return '-';
    } else {
        switch (format->sign) {
            case KALOS_STRING_FORMAT_SIGN_NEGATIVE:
                return 0;
            case KALOS_STRING_FORMAT_SIGN_POSITIVE_SPACE:
                return ' ';
            case KALOS_STRING_FORMAT_SIGN_BOTH:
                return '+';
        }
    }
    return 0;
}

char* kalos_string_format_int(kalos_int value, kalos_string_format* format, kalos_mem_alloc alloc) {
    if (format->align == KALOS_STRING_FORMAT_ALIGN_PADDING_AFTER_SIGN) {
        // TODO
        return NULL;
    }
    
    char string_format[10];
    char* fmt = &string_format[0];
    char sign = get_sign(format, value);
    value = abs(value);
    int width = format->align == KALOS_STRING_FORMAT_ALIGN_CENTER ? 0 : format->min_width;
    if (sign) {
        *fmt++ = sign;
        width--;
    }
    const char* alt_fmt = get_alt_format(format);
    if (alt_fmt[0]) {
        *fmt++ = alt_fmt[0];
        *fmt++ = alt_fmt[1];
        width -= 2;
    }
    width = max(width, 0);
    if (format->align == KALOS_STRING_FORMAT_ALIGN_LEFT) {
        width = -width;
    }
    *fmt++ = '%';
    *fmt++ = format->type ? format->type : 'd';
    *fmt++ = 0;
    int size = snprintf(NULL, 0, string_format, value);
    int offset = 0;
    if (size < format->min_width) {
        if (format->align == KALOS_STRING_FORMAT_ALIGN_CENTER) {
            offset = (format->min_width - size) / 2;
        } else if (format->align == KALOS_STRING_FORMAT_ALIGN_RIGHT) {
            offset = format->min_width - size;
        }
        char* s = alloc(format->min_width + 1);
        memset(s, format->fill ? format->fill : ' ', format->min_width);
        sprintf(s + offset, string_format, value);
        if (format->align != KALOS_STRING_FORMAT_ALIGN_RIGHT) {
            s[offset + size] = format->fill ? format->fill : ' ';
            s[format->min_width] = 0;
        }
        size = format->min_width;
        return s;
    } else {
        char* s = alloc(size + 1);
        sprintf(s, string_format, value);
        return s;
    }
}
