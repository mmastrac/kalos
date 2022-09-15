#pragma once

#include <stdlib.h>
#include <string.h>

#ifdef __WATCOMC__
#pragma off (unreferenced)
#pragma on (reuse_duplicate_strings)
#else
// Allow modern IDEs to type-check our code
#ifndef __huge
#define __huge
#endif
#ifndef __far
#define __far
#endif
// These macros suck, but watcom provides them
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifdef IS_TEST
#include <assert.h>
#define ASSERT(...) assert(__VA_ARGS__)
void test_log(const char* msg, ...);
#define LOG(...) {test_log(__FILE__, __LINE__, __VA_ARGS__);}
#else
// #define ASSERT(x) { if (!(x)) { printf("%s", "ASSERTION FAILED: " #x "\n"); } else { printf("%s", "OK " #x "\n"); } }
void log_printf(const char* msg, ...);
#ifdef DEBUG
#define ASSERT(x) { if (!(x)) { printf("%s:%d %s", __FILE__, __LINE__, "ASSERTION FAILED: " #x "\n"); } }
#ifndef LOG
#define LOG(...) log_printf(__VA_ARGS__)
#endif
#else
#define ASSERT(x) if (sizeof(x)) {}
#define LOG(...)
#endif

#endif

#define PTR_BYTE_OFFSET(ptr, offset) (void*)(((uint8_t*)ptr)+(ptrdiff_t)offset)
#define PTR_BYTE_OFFSET_NEG(ptr, offset) (void*)(((uint8_t*)ptr)-(ptrdiff_t)offset)
#define PTR_BYTE_SUBTRACT(ptr1, ptr2) (((uint8_t*)ptr1) - ((uint8_t*)ptr2))
