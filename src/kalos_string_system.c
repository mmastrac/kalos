#include <stdlib.h>
#include "../defines.h"
#define KALOS_STRING_SYSTEM_C
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
        str = __kalos_string_alloc(state, size);
        char* s = __kalos_string_data(str);
        sprintf(s, string_format, value);
        VALIDATE_STRING(str);
        return str;
    }
}

// Adapted from https://creativeandcritical.net/str-replace-c
static kalos_string repl_str(kalos_state state, kalos_string s, const char *from, const char *to) {
	/* Adjust each of the below values to suit your needs. */

	/* Increment positions cache size initially by this number. */
	size_t cache_sz_inc = 16;
	/* Thereafter, each time capacity needs to be increased,
	 * multiply the increment by this factor. */
	const size_t cache_sz_inc_factor = 3;
	/* But never increment capacity by more than this number. */
	const size_t cache_sz_inc_max = 1048576;

	char *pret, *ret = NULL;
    kalos_writable_string w;
    const char *str = __kalos_string_data(s);
	const char *pstr2, *pstr = str;
	size_t i, count = 0;
	#if (__STDC_VERSION__ >= 199901L)
	uintptr_t *pos_cache_tmp, *pos_cache = NULL;
	#else
	ptrdiff_t *pos_cache_tmp, *pos_cache = NULL;
	#endif
	size_t cache_sz = 0;
	ssize_t cpylen, orglen, retlen, tolen, fromlen = strlen(from);

	/* Find all matches and cache their positions. */
	while ((pstr2 = strstr(pstr, from)) != NULL) {
		count++;

		/* Increase the cache size when necessary. */
		if (cache_sz < count) {
			cache_sz += cache_sz_inc;
			pos_cache_tmp = kalos_mem_realloc(state, pos_cache, sizeof(*pos_cache) * cache_sz);
			if (pos_cache_tmp == NULL) {
				goto end_repl_str;
			} else pos_cache = pos_cache_tmp;
			cache_sz_inc *= cache_sz_inc_factor;
			if (cache_sz_inc > cache_sz_inc_max) {
				cache_sz_inc = cache_sz_inc_max;
			}
		}

		pos_cache[count-1] = pstr2 - str;
		pstr = pstr2 + fromlen;
	}

	orglen = pstr - str + strlen(pstr);
    if (count == 0) {
        return s;
    }

	/* Allocate memory for the post-replacement string. */
    tolen = strlen(to);
    retlen = orglen + (tolen - fromlen) * (ssize_t)count;
    w = kalos_string_allocate_writable_size(state, retlen);
	ret = kalos_string_writable_c(state, w);
	if (ret == NULL) {
		goto end_repl_str;
	}

    /* Duplicate the string whilst performing the replacements using the position cache. */
    pret = ret;
    memcpy(pret, str, pos_cache[0]);
    pret += pos_cache[0];
    for (i = 0; i < count; i++) {
        memcpy(pret, to, tolen);
        pret += tolen;
        pstr = str + pos_cache[i] + fromlen;
        cpylen = (i == count-1 ? orglen : pos_cache[i+1]) - pos_cache[i] - fromlen;
        memcpy(pret, pstr, cpylen);
        pret += cpylen;
    }
    ret[retlen] = '\0';

end_repl_str:
	/* Free the cache and return the post-replacement string,
	 * which will be NULL in the event of an error. */
	kalos_mem_free(state, pos_cache);
    kalos_string_release(state, s);
	return kalos_string_commit_length(state, w, retlen);
}

kalos_string kalos_string_take_replace(kalos_state state, kalos_string* s_, kalos_string* a_, kalos_string* b_) {
    kalos_string s = kalos_string_take(state, s_);
    kalos_string a = kalos_string_take(state, a_);
    kalos_string b = kalos_string_take(state, b_);
    s = repl_str(state, s, __kalos_string_data(a), __kalos_string_data(b));
    kalos_string_release(state, a);
    kalos_string_release(state, b);
    VALIDATE_STRING(s);
    return s;
}
