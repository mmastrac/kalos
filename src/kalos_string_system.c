#include <stdlib.h>
#include "../defines.h"
#include "kalos_string_system.h"

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

kalos_string kalos_string_format_int(kalos_state state, kalos_int value, kalos_string_format* format) {
    kalos_string str;
    if (format->align == KALOS_STRING_FORMAT_ALIGN_PADDING_AFTER_SIGN) {
        // TODO
        return kalos_string_allocate(state, "");
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
        str = __kalos_string_alloc(state, format->min_width);
        char* s = __kalos_string_data(str);
        memset(s, format->fill ? format->fill : ' ', format->min_width);
        sprintf(s + offset, string_format, value);
        if (format->align != KALOS_STRING_FORMAT_ALIGN_RIGHT) {
            s[offset + size] = format->fill ? format->fill : ' ';
            s[format->min_width] = 0;
        }
        VALIDATE_STRING(str);
        return str;
    } else {
        return kalos_string_allocate_fmt(state, string_format, value);
        str = __kalos_string_alloc(state, size);
        char* s = __kalos_string_data(str);
        sprintf(s, string_format, value);
        VALIDATE_STRING(str);
        return str;
    }
}

void kalos_string_release(kalos_state state, kalos_string s) {
    if (s.length__ > 0) {
        if (s.sa->count == 0) {
            s.sa->count = KALOS_STRING_POISONED;
            kalos_mem_free(state, (void*)s.sa);
        } else {
            s.sa->count--;
        }
    }
}
