#pragma once
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "kalos.h"
#include "kalos_string_format.h"

#ifdef IS_TEST
#define VALIDATE_STRING(str_) { ASSERT(strlen(str_.s) == str_.length); ASSERT(str_.s[str_.length] == 0); }
#else
#define VALIDATE_STRING(str_)
#endif

/*
    All functions that end in take_ assume that the string is now owned by the function. Callers must
    duplicate the string if they wish to save a copy. All other functions borrow the reference held by
    the caller.
*/

static inline kalos_string kalos_string_allocate(kalos_state state, const char* string);
static inline kalos_string kalos_string_allocate_fmt(kalos_state state, const char* fmt, ...);
static inline kalos_writable_string kalos_string_allocate_writable(kalos_state state, const char* string);
static inline kalos_writable_string kalos_string_allocate_writable_size(kalos_state state, int size);

kalos_string kalos_string_duplicate(kalos_state state, kalos_string s);
kalos_writable_string kalos_string_duplicate_writable(kalos_state state);
static inline kalos_string kalos_string_commit(kalos_state state, kalos_writable_string string);
void kalos_string_release(kalos_state state, kalos_string string);

static inline char* kalos_string_writable_c(kalos_state state, kalos_writable_string s) { return s.s; }
static inline const char* kalos_string_c(kalos_state state, kalos_string s) { return s.s; }
static inline char kalos_string_char_at(kalos_state state, kalos_string s, kalos_int index) { return s.s[index]; }

static inline bool kalos_string_isempty(kalos_state state, kalos_string string) { return string.length == 0; }
static inline kalos_int kalos_string_length(kalos_state state, kalos_string string) { return string.length; }
static inline kalos_int kalos_string_compare(kalos_state state, kalos_string a, kalos_string b);
static inline kalos_int kalos_string_find(kalos_state state, kalos_string a, kalos_string b);
static inline kalos_int kalos_string_find_from(kalos_state state, kalos_string a, kalos_string b, int start);

static inline kalos_string kalos_string_take_substring(kalos_state state, kalos_string string, int start, int length);
static inline kalos_string kalos_string_take_substring_start(kalos_state state, kalos_string string, int start);
static inline kalos_string kalos_string_take_append(kalos_state state, kalos_string a, kalos_string b);
static inline kalos_string kalos_string_take_repeat(kalos_state state, kalos_string a, kalos_int b);

kalos_string kalos_string_format_int(kalos_state state, kalos_int value, kalos_string_format* string_format);

static inline kalos_string kalos_string_allocate(kalos_state state, const char* string) {
    kalos_string s = { .count = 0 };
    s.s = kalos_mem_alloc(state, strlen(string) + 1);
    s.length = strlen(string);
    strcpy((char*)s.s, string);
    VALIDATE_STRING(s);
    return s;
}

static inline kalos_string kalos_string_allocate_fmt(kalos_state state, const char* fmt, ...) {
    kalos_string s = { .count = 0 };

    va_list args;
    va_start(args, fmt);
    int size = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    s.s = kalos_mem_alloc(state, size + 1);
    va_start(args, fmt);
    s.length = vsprintf((char*)s.s, fmt, args);
    va_end(args);
    VALIDATE_STRING(s);
    return s;
}

static inline kalos_writable_string kalos_string_allocate_writable_size(kalos_state state, int size) {
    kalos_writable_string s;
    s.s = kalos_mem_alloc(state, size + 1);
    s.s[size] = 0;
    return s;
}

static inline kalos_string kalos_string_commit(kalos_state state, kalos_writable_string string) {
    kalos_string s = { .count = 0 };
    s.length = strlen(string.s);
    s.s = string.s;
    VALIDATE_STRING(s);
    return s;
}

static inline kalos_int kalos_string_compare(kalos_state state, kalos_string a, kalos_string b) {
    return strcmp(a.s, b.s);
}

static inline kalos_int kalos_string_find(kalos_state state, kalos_string a, kalos_string b) {
    char* found = strstr(a.s, b.s);
    if (!found) {
        return -1;
    }
    return found - a.s;
}

static inline kalos_int kalos_string_find_from(kalos_state state, kalos_string a, kalos_string b, int start) {
    char* found = strstr(a.s + start, b.s);
    if (!found) {
        return -1;
    }
    return found - a.s;
}

static inline kalos_string kalos_string_take_substring(kalos_state state, kalos_string string, int start, int length) {
    kalos_string str = { .count = 0 };
    str.length = length;
    char* s = kalos_mem_alloc(state, length + 1);
    str.s = s;
    memcpy((char*)s, string.s + start, length);
    s[length] = 0;
    VALIDATE_STRING(str);
    return str;
}

static inline kalos_string kalos_string_take_substring_start(kalos_state state, kalos_string string, int start) {
    return kalos_string_take_substring(state, string, start, string.length - start);
}

static inline kalos_string kalos_string_take_append(kalos_state state, kalos_string a, kalos_string b) {
    kalos_string str;
    str.count = 0;
    str.length = a.length + b.length;
    char* s = kalos_mem_alloc(state, a.length + b.length + 1);
    str.s = s;
    memcpy(s, a.s, a.length);
    memcpy(s + a.length, b.s, b.length);
    s[str.length] = 0;
    VALIDATE_STRING(str);
    return str;
}

static inline kalos_string kalos_string_take_repeat(kalos_state state, kalos_string a, kalos_int b) {
    kalos_string str;
    str.count = 0;
    str.length = a.length * b;
    char* s = kalos_mem_alloc(state, a.length * b + 1);
    str.s = s;
    for (int i = 0; i < b; i++) {
        memcpy(s, a.s, a.length);
        s += a.length;
    }
    s[str.length] = 0;
    VALIDATE_STRING(str);
    return str;
}
