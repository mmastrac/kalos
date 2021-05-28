#pragma once
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../defines.h"
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

/*
    All functions that end in take_ assume that the string is now owned by the function. Callers must
    duplicate the string if they wish to save a copy. All other functions borrow the reference held by
    the caller.
*/

static inline char* __kalos_string_data(kalos_string s) {
    return s.length__ <= 0 ? (char*)s.sc : ((char*)s.sa + sizeof(kalos_string_allocated));
}

static inline kalos_string kalos_string_allocate(kalos_state state, const char* string);
static inline kalos_string kalos_string_allocate_fmt(kalos_state state, const char* fmt, ...);
static inline kalos_writable_string kalos_string_allocate_writable(kalos_state state, const char* string);
static inline kalos_writable_string kalos_string_allocate_writable_size(kalos_state state, int size);

static inline kalos_string kalos_string_duplicate(kalos_state state, kalos_string s);
kalos_writable_string kalos_string_duplicate_writable(kalos_state state);
static inline kalos_string kalos_string_commit(kalos_state state, kalos_writable_string string);
void kalos_string_release(kalos_state state, kalos_string string);

static inline char* kalos_string_writable_c(kalos_state state, kalos_writable_string s) { return (char*)s.s + sizeof(kalos_string_allocated); }
static inline const char* kalos_string_c(kalos_state state, kalos_string s);
static inline char kalos_string_char_at(kalos_state state, kalos_string s, kalos_int index) { return __kalos_string_data(s)[index]; }

static inline bool kalos_string_isempty(kalos_state state, kalos_string string) { return string.length__ == 0; }
static inline kalos_int kalos_string_length(kalos_state state, kalos_string string) { return abs(string.length__); }
static inline kalos_int kalos_string_compare(kalos_state state, kalos_string a, kalos_string b);
static inline kalos_int kalos_string_find(kalos_state state, kalos_string a, kalos_string b);
static inline kalos_int kalos_string_find_from(kalos_state state, kalos_string a, kalos_string b, int start);

static inline kalos_string kalos_string_take(kalos_state state, kalos_string* string);
static inline kalos_string kalos_string_take_substring(kalos_state state, kalos_string* string, int start, int length);
static inline kalos_string kalos_string_take_substring_start(kalos_state state, kalos_string* string, int start);
static inline kalos_string kalos_string_take_append(kalos_state state, kalos_string* a, kalos_string* b);
static inline kalos_string kalos_string_take_repeat(kalos_state state, kalos_string* a, kalos_int b);

kalos_string kalos_string_format_int(kalos_state state, kalos_int value, kalos_string_format* string_format);

static inline kalos_string __kalos_string_alloc(kalos_state state, kalos_int size) {
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

static inline kalos_string kalos_string_allocate(kalos_state state, const char* string) {
    kalos_string s;
    s.sc = string;
    s.length__ = -(int)strlen(string);
    VALIDATE_STRING(s);
    return s;
}

static inline kalos_string kalos_string_allocate_fmt(kalos_state state, const char* fmt, ...) {
    kalos_string s;

    va_list args;
    va_start(args, fmt);
    int size = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    s = __kalos_string_alloc(state, size);
    va_start(args, fmt);
    vsprintf(__kalos_string_data(s), fmt, args);
    va_end(args);
    VALIDATE_STRING(s);
    return s;
}

static inline kalos_writable_string kalos_string_allocate_writable_size(kalos_state state, int size) {
    kalos_writable_string s;
    s.s = kalos_mem_alloc(state, size + sizeof(kalos_string_allocated) + 1);
    kalos_string_writable_c(state, s)[size] = 0;
    return s;
}

static inline kalos_string kalos_string_duplicate(kalos_state state, kalos_string s) {
    VALIDATE_STRING(s);
    if (s.length__ <= 0) {
        return s;
    } else {
        s.sa->count++;
        return s;
    }
}

static inline kalos_string kalos_string_commit(kalos_state state, kalos_writable_string string) {
    kalos_string s;
    s.length__ = strlen(kalos_string_writable_c(state, string));
    s.sa = string.s;
    s.sa->count = 0;
    VALIDATE_STRING(s);
    return s;
}

static inline const char* kalos_string_c(kalos_state state, kalos_string s) {
    VALIDATE_STRING(s);
    return __kalos_string_data(s);
}

static inline kalos_int kalos_string_compare(kalos_state state, kalos_string a, kalos_string b) {
    return strcmp(__kalos_string_data(a), __kalos_string_data(b));
}

static inline kalos_int kalos_string_find(kalos_state state, kalos_string a, kalos_string b) {
    char* found = strstr(__kalos_string_data(a), __kalos_string_data(b));
    if (!found) {
        return -1;
    }
    return found - __kalos_string_data(a);
}

static inline kalos_int kalos_string_find_from(kalos_state state, kalos_string a, kalos_string b, int start) {
    char* found = strstr(__kalos_string_data(a) + start, __kalos_string_data(b));
    if (!found) {
        return -1;
    }
    return found - __kalos_string_data(a);
}

static inline kalos_string kalos_string_take(kalos_state state, kalos_string* string) {
    kalos_string copy = *string;
    string->length__ = 0;
    string->sc = "";
    return copy;
}

static inline kalos_string kalos_string_take_substring(kalos_state state, kalos_string* string_, int start, int length) {
    kalos_string string = kalos_string_take(state, string_);
    kalos_string str = __kalos_string_alloc(state, length);
    char* s = __kalos_string_data(str);
    if (length) {
        memcpy(s, __kalos_string_data(string) + start, length);
        s[length] = 0;
    }
    VALIDATE_STRING(str);
    kalos_string_release(state, str);
    return str;
}

static inline kalos_string kalos_string_take_substring_start(kalos_state state, kalos_string* string, int start) {
    return kalos_string_take_substring(state, string, start, kalos_string_length(state, *string) - start);
}

static inline kalos_string kalos_string_take_append(kalos_state state, kalos_string* a_, kalos_string* b_) {
    kalos_string a = kalos_string_take(state, a_);
    kalos_string b = kalos_string_take(state, b_);
    kalos_int al = kalos_string_length(state, a);
    kalos_int bl = kalos_string_length(state, b);
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

static inline kalos_string kalos_string_take_repeat(kalos_state state, kalos_string* a_, kalos_int b) {
    kalos_string a = kalos_string_take(state, a_);
    kalos_int al = kalos_string_length(state, a);
    kalos_string str = __kalos_string_alloc(state, al * b);
    char* s = __kalos_string_data(str);
    char* as = __kalos_string_data(a);
    for (int i = 0; i < b; i++) {
        memcpy(s, as, al);
        s += al;
    }
    s[al * b] = 0;
    VALIDATE_STRING(str);
    kalos_string_release(state, a);
    return str;
}
