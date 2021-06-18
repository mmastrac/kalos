#pragma once
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"
#include "kalos.h"
#include "kalos_string_format.h"

#pragma warning 303 9

#define KALOS_STRING_POISONED 0x7fff

#ifdef IS_TEST
#define VALIDATE_STRING(str_) { \
    ASSERT(strlen(__kalos_string_data(str_)) == abs(str_.length__)); \
    ASSERT(__kalos_string_data(str_)[abs(str_.length__)] == 0); \
    ASSERT(str_.length__ <= 0 || str_.sa->count != KALOS_STRING_POISONED); \
}
#else
#define VALIDATE_STRING(str_)
#endif

// #define KALOS_STRING_AGGRESSIVE_INLINE 1

#if !KALOS_STRING_AGGRESSIVE_INLINE
#ifdef KALOS_STRING_SYSTEM_C
#define KALOS_STRING_INLINE 
#else
#define KALOS_STRING_INLINE extern
#endif
#else
#define KALOS_STRING_INLINE static inline
#endif

#define KALOS_STRING_ALWAYS_INLINE static inline

/*
    All functions that end in take_ assume that the string is now owned by the function. Callers must
    duplicate the string if they wish to save a copy. All other functions borrow the reference held by
    the caller.
*/

KALOS_STRING_ALWAYS_INLINE char* __kalos_string_data(kalos_string s) {
    return s.length__ <= 0 ? (char*)s.sc : ((char*)s.sa + sizeof(kalos_string_allocated));
}

KALOS_STRING_INLINE kalos_string kalos_string_allocate(kalos_state state, const char* string);
KALOS_STRING_INLINE kalos_string kalos_string_allocate_fmt(kalos_state state, const char* fmt, ...);
KALOS_STRING_INLINE kalos_writable_string kalos_string_allocate_writable(kalos_state state, const char* string);
KALOS_STRING_INLINE kalos_writable_string kalos_string_allocate_writable_size(kalos_state state, int size);

KALOS_STRING_INLINE kalos_string kalos_string_duplicate(kalos_state state, kalos_string s);
KALOS_STRING_INLINE kalos_string kalos_string_commit(kalos_state state, kalos_writable_string string);
KALOS_STRING_INLINE void kalos_string_release(kalos_state state, kalos_string string);

KALOS_STRING_ALWAYS_INLINE char* kalos_string_writable_c(kalos_state state, kalos_writable_string s) { return (char*)s.s + sizeof(kalos_string_allocated); }
KALOS_STRING_INLINE const char* kalos_string_c(kalos_state state, kalos_string s);
KALOS_STRING_ALWAYS_INLINE char kalos_string_char_at(kalos_state state, kalos_string s, kalos_int index) { return __kalos_string_data(s)[index]; }

KALOS_STRING_ALWAYS_INLINE bool kalos_string_isempty(kalos_state state, kalos_string string) { return string.length__ == 0; }
KALOS_STRING_ALWAYS_INLINE kalos_int kalos_string_length(kalos_state state, kalos_string string) { return abs(string.length__); }
KALOS_STRING_INLINE kalos_int kalos_string_compare(kalos_state state, kalos_string a, kalos_string b);
KALOS_STRING_INLINE kalos_int kalos_string_find(kalos_state state, kalos_string a, kalos_string b);
KALOS_STRING_INLINE kalos_int kalos_string_find_from(kalos_state state, kalos_string a, kalos_string b, int start);

KALOS_STRING_INLINE kalos_string kalos_string_take(kalos_state state, kalos_string* string);
KALOS_STRING_INLINE kalos_string kalos_string_take_substring(kalos_state state, kalos_string* string, int start, int length);
KALOS_STRING_ALWAYS_INLINE kalos_string kalos_string_take_substring_start(kalos_state state, kalos_string* string, int start) {
    return kalos_string_take_substring(state, string, start, kalos_string_length(state, *string) - start);
}
KALOS_STRING_INLINE kalos_string kalos_string_take_append(kalos_state state, kalos_string* a, kalos_string* b);
KALOS_STRING_INLINE kalos_string kalos_string_take_repeat(kalos_state state, kalos_string* a, kalos_int b);

kalos_string kalos_string_format_int(kalos_state state, kalos_int value, kalos_string_format* string_format);
kalos_string kalos_string_take_replace(kalos_state state, kalos_string* s, kalos_string* a, kalos_string* b);

#if defined(KALOS_STRING_SYSTEM_C) || KALOS_STRING_AGGRESSIVE_INLINE

KALOS_STRING_INLINE kalos_string __kalos_string_alloc(kalos_state state, kalos_int size) {
    kalos_string s;
    s.length__ = size;
    if (size) {
        s.sa = kalos_mem_alloc(state, size + sizeof(kalos_string_allocated) + 1);
        s.sa->count = 0;
    } else {
        s.sc = "";
    }
    return s;
}

KALOS_STRING_INLINE kalos_string kalos_string_allocate(kalos_state state, const char* string) {
#ifdef KALOS_STRING_TORTURE_TEST
    return kalos_string_allocate_fmt(state, "%s", string);
#else
    kalos_string s;
    s.sc = string;
    s.length__ = -(int)strlen(string);
    VALIDATE_STRING(s);
    return s;
#endif
}

KALOS_STRING_INLINE kalos_string kalos_string_allocate_fmt(kalos_state state, const char* fmt, ...) {
    kalos_string s;

    va_list args;
    va_start(args, fmt);
    int size = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    s = __kalos_string_alloc(state, size);
    if (size) {
        va_start(args, fmt);
        vsprintf(__kalos_string_data(s), fmt, args);
        va_end(args);
    }
    VALIDATE_STRING(s);
    return s;
}

KALOS_STRING_INLINE kalos_writable_string kalos_string_allocate_writable_size(kalos_state state, int size) {
    kalos_writable_string s;
    s.s = kalos_mem_alloc(state, size + sizeof(kalos_string_allocated) + 1);
    kalos_string_writable_c(state, s)[size] = 0;
    return s;
}

KALOS_STRING_INLINE kalos_string kalos_string_duplicate(kalos_state state, kalos_string s) {
    VALIDATE_STRING(s);
    if (s.length__ <= 0) {
        return s;
    } else {
        s.sa->count++;
        return s;
    }
}

KALOS_STRING_INLINE kalos_string kalos_string_commit(kalos_state state, kalos_writable_string string) {
    kalos_string s;
    s.length__ = strlen(kalos_string_writable_c(state, string));
    s.sa = string.s;
    s.sa->count = 0;
    VALIDATE_STRING(s);
    return s;
}

KALOS_STRING_INLINE kalos_string kalos_string_commit_length(kalos_state state, kalos_writable_string string, int length) {
    kalos_string s;
    s.length__ = length;
    s.sa = string.s;
    s.sa->count = 0;
    VALIDATE_STRING(s);
    return s;
}

KALOS_STRING_INLINE void kalos_string_release(kalos_state state, kalos_string s) {
    if (s.length__ > 0) {
        if (s.sa->count == 0) {
            s.sa->count = KALOS_STRING_POISONED;
            kalos_mem_free(state, (void*)s.sa);
        } else {
            ASSERT(s.sa->count != KALOS_STRING_POISONED);
            s.sa->count--;
        }
    }
}

KALOS_STRING_INLINE const char* kalos_string_c(kalos_state state, kalos_string s) {
    VALIDATE_STRING(s);
    return __kalos_string_data(s);
}

KALOS_STRING_INLINE kalos_int kalos_string_compare(kalos_state state, kalos_string a, kalos_string b) {
    return strcmp(__kalos_string_data(a), __kalos_string_data(b));
}

KALOS_STRING_INLINE kalos_int kalos_string_find(kalos_state state, kalos_string a, kalos_string b) {
    char* found = strstr(__kalos_string_data(a), __kalos_string_data(b));
    if (!found) {
        return -1;
    }
    return found - __kalos_string_data(a);
}

KALOS_STRING_INLINE kalos_int kalos_string_find_from(kalos_state state, kalos_string a, kalos_string b, int start) {
    char* found = strstr(__kalos_string_data(a) + start, __kalos_string_data(b));
    if (!found) {
        return -1;
    }
    return found - __kalos_string_data(a);
}

KALOS_STRING_INLINE kalos_string kalos_string_take(kalos_state state, kalos_string* string) {
    kalos_string copy = *string;
    string->length__ = 0;
    string->sc = "";
    return copy;
}

KALOS_STRING_INLINE kalos_string kalos_string_take_substring(kalos_state state, kalos_string* string_, int start, int length) {
    kalos_string string = kalos_string_take(state, string_);
    kalos_string str = __kalos_string_alloc(state, length);
    char* s = __kalos_string_data(str);
    if (length) {
        memcpy(s, __kalos_string_data(string) + start, length);
        s[length] = 0;
    }
    kalos_string_release(state, string);
    VALIDATE_STRING(str);
    return str;
}

KALOS_STRING_INLINE kalos_string kalos_string_take_append(kalos_state state, kalos_string* a_, kalos_string* b_) {
    kalos_string a = kalos_string_take(state, a_);
    kalos_string b = kalos_string_take(state, b_);
    kalos_int al = kalos_string_length(state, a);
    kalos_int bl = kalos_string_length(state, b);
    if (al == 0) {
        kalos_string_release(state, a);
        return b;
    } else if (bl == 0) {
        kalos_string_release(state, b);
        return a;
    }
    kalos_string str = __kalos_string_alloc(state, al + bl);
    char* s = __kalos_string_data(str);
    memcpy(s, __kalos_string_data(a), al);
    memcpy(s + al, __kalos_string_data(b), bl);
    s[al + bl] = 0;
    VALIDATE_STRING(str);
    kalos_string_release(state, a);
    kalos_string_release(state, b);
    return str;
}

KALOS_STRING_INLINE kalos_string kalos_string_take_repeat(kalos_state state, kalos_string* a_, kalos_int b) {
    kalos_string a = kalos_string_take(state, a_);
    kalos_int al = kalos_string_length(state, a);
    ASSERT(b > 0 && al > 0);
    kalos_string str = __kalos_string_alloc(state, al * b);
    char* s = __kalos_string_data(str);
    char* as = __kalos_string_data(a);
    for (int i = 0; i < b; i++) {
        memcpy(s, as, al);
        s += al;
    }
    *s = 0;
    kalos_string_release(state, a);
    VALIDATE_STRING(str);
    return str;
}
#endif
